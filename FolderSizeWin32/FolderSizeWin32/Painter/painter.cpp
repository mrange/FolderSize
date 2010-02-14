/* ****************************************************************************
 *
 * Copyright (c) M�rten R�nge.
 *
 * This source code is subject to terms and conditions of the Microsoft Public License. A 
 * copy of the license can be found in the License.html folder_infole at the root of this distribution. If 
 * you cannot locate the  Microsoft Public License, please send an email to 
 * dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
 * by the terms of the Microsoft Public License.
 *
 * You must not remove this notice, or any other, from this software.
 *
 *
 * ***************************************************************************/

// ----------------------------------------------------------------------------
#include "stdafx.h"
// ----------------------------------------------------------------------------
#undef max
#undef min
// ----------------------------------------------------------------------------
#include "../linear.hpp"
#include "../messages.hpp"
#include "../utility.hpp"
#include "../theme.hpp"
#include "../view_transform.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
#include "painter.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace b    = boost           ;
   namespace f    = folder          ;
   namespace s    = std             ;
   namespace st   = std::tr1        ;
   namespace w    = win32           ;
   namespace l    = linear          ;
   namespace vt   = view_transform  ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      std::size_t const buffer_size    = 256;
      double const      cut_off_y      = 10.0;

      struct update_request : boost::noncopyable
      {
         typedef std::auto_ptr<update_request> ptr;

         update_request (
               folder::folder const * const  root_
            ,  HWND const                    main_hwnd_
            ,  HDC const                     hdc_
            ,  std::size_t const             processed_folder_count_
            ,  std::size_t const             unprocessed_folder_count_
            ,  coordinate const &            centre_        
            ,  zoom_factor const &           zoom_          
            ,  dimension const &             screen_size_   
            )
            :  root                       (root_                           )  
            ,  main_hwnd                  (main_hwnd_                      )
            ,  processed_folder_count     (processed_folder_count_         )
            ,  unprocessed_folder_count   (unprocessed_folder_count_       )
            ,  centre                     (centre_                         )
            ,  zoom                       (zoom_                           )
            ,  bitmap_size                (screen_size_                    )
            ,  bitmap_planes              (GetDeviceCaps (hdc_, BITSPIXEL) )
            ,  bitmap_bits                (GetDeviceCaps (hdc_, PLANES)    )
         {                                
         }

         folder::folder const * const  root                    ;

         HWND const                    main_hwnd               ;
         std::size_t const             processed_folder_count  ;
         std::size_t const             unprocessed_folder_count;
         coordinate const              centre                  ;
         zoom_factor const             zoom                    ;
         dimension const               bitmap_size             ;
         int const                     bitmap_planes           ;
         int const                     bitmap_bits             ;
      };

      HBITMAP const create_bitmap (
            int const bitmap_planes
         ,  int const bitmap_bits
         ,  dimension const & bitmap_size
         )
      {
         auto cx = bitmap_size.x ();
         auto cy = bitmap_size.y ();

         return 
            CreateBitmap (
                  IMPLICIT_CAST (cx)
               ,  IMPLICIT_CAST (cy)
               ,  bitmap_planes
               ,  bitmap_bits
               ,  NULL);
      }

      struct update_response : boost::noncopyable
      {
         typedef std::auto_ptr<update_response> ptr;

         update_response (
            update_request    &  update_request_
            )
            :  centre      (update_request_.centre             )
            ,  zoom        (update_request_.zoom               )
            ,  bitmap_size (update_request_.bitmap_size        )
            ,  bitmap      (create_bitmap (
                  update_request_.bitmap_planes
               ,  update_request_.bitmap_bits
               ,  update_request_.bitmap_size               )  )
         {
         }

         coordinate const                 centre            ;
         zoom_factor const                zoom              ;
         dimension const                  bitmap_size       ;
         win32::gdi_object<HBITMAP> const bitmap            ;
      };

      struct painter_context
      {
         painter_context (
               HDC const hdc_
            ,  HBRUSH const fill_brush_
            ,  HBRUSH const frame_brush_
            )
            :  hdc(hdc_)
            ,  fill_brush(fill_brush_)
            ,  frame_brush(frame_brush_)
         {
         }

         HDC const            hdc               ;
         HBRUSH const         fill_brush        ;
         HBRUSH const         frame_brush       ;
      };

      struct background_painter
      {
         background_painter ()
            :  thread            (_T("painter"), create_proc ())
            ,  new_frame_request (w::event_type::auto_reset    )
            ,  shutdown_request  (w::event_type::manual_reset  )
         {
         }

         ~background_painter () throw ()
         {
            shutdown_request.set ();
            thread.join (10000);
         }

         w::thread_safe_scoped_ptr<update_request>    update_request_value    ;
         w::thread_safe_scoped_ptr<update_response>   update_response_value   ;
         w::event                                     new_frame_request       ;

      private:
         w::thread::proc create_proc () throw ()
         {
            return st::bind (&background_painter::proc, this);
         }

         template<
               typename TPropertyPickerPredicate
            ,  typename TPainterPredicate>
         static void folder_traverser_impl (
               std::size_t                remaining_levels
            ,  dimension const &          size
            ,  vt::transform const &      transform
            ,  double const               x
            ,  double const               y
            ,  double const               x_step_ratio
            ,  double const               y_step_ratio
            ,  f::folder const * const    folder
            ,  TPropertyPickerPredicate   property_picker
            ,  TPainterPredicate          painter
            )
         {
            if (remaining_levels == 0)
            {
               return;
            }

            if (!folder)
            {
               return;
            }

            auto property = property_picker (folder);

            auto current_x = x;
            auto current_y = y;
            auto next_x = current_x + x_step_ratio;
            auto height = property * y_step_ratio;

            auto current_left_top = transform * vt::create_extended_vector (
                  current_x
               ,  current_y
               );
            auto current_right_bottom = transform * vt::create_extended_vector (
                  current_x + x_step_ratio
                , current_y + height
                );

            auto adjusted_height = current_right_bottom.y () - current_left_top.y ();

            if (adjusted_height < cut_off_y)
            {
               return;
            }

            if (
                  current_right_bottom.x () >= 0.0
               && current_right_bottom.y () >= 0.0
               && current_left_top.x () < size.x ()
               && current_left_top.y () < size.y ()
               )
            {
               painter (
                     property
                  ,  current_left_top.x ()
                  ,  current_left_top.y ()
                  ,  current_right_bottom.x ()
                  ,  current_right_bottom.y ()
                  ,  *folder
                  );
            }
               
            auto folder_count = folder->folder_count;

            for (std::size_t iter = 0; iter < folder_count; ++iter)
            {
               auto sub_folder = folder->sub_folders[iter];

               if (!sub_folder)
               {
                  continue;
               }

               auto sub_property = property_picker (sub_folder);
               auto y_step = y_step_ratio * sub_property;

               folder_traverser_impl (
                     remaining_levels - 1
                  ,  size
                  ,  transform
                  ,  next_x
                  ,  current_y
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  sub_folder
                  ,  property_picker 
                  ,  painter
                  );

               current_y += y_step;
            }
         }

         template<
               typename TPropertyPickerPredicate
            ,  typename TPainterPredicate>
         static void folder_traverser (
               dimension const &          size
            ,  vt::transform const &      transform
            ,  f::folder const * const    root
            ,  TPropertyPickerPredicate   property_picker
            ,  TPainterPredicate          painter
            )
         {
            if (! (root && size.x () > 0 && size.y () > 0) )
            {
               return;
            }

            auto depth     = root->get_depth ();
            auto property  = property_picker (root);

            if (property > 0 && depth > 0)
            {
               auto x_step_ratio = size.x () / depth;
               auto y_step_ratio = size.y () / property;

               folder_traverser_impl (
                     depth
                  ,  size
                  ,  transform
                  ,  0.0
                  ,  0.0
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  root
                  ,  property_picker
                  ,  painter
                  );
            }
         }

         static big_size size_picker(
            f::folder const * f
            )
         {
            return f->get_total_size ();
         }

         static void painter (
               painter_context const & painter_context
            ,  big_size const  total_size
            ,  double const            left
            ,  double const            top
            ,  double const            right
            ,  double                  bottom
            ,  f::folder const &       folder
            )
         {
            RECT rect         = {0}                      ;
            rect.left         = IMPLICIT_CAST (left   )  ;
            rect.top          = IMPLICIT_CAST (top    )  ;
            rect.right        = IMPLICIT_CAST (right  )  ;
            rect.bottom       = IMPLICIT_CAST (bottom )  ;

            TCHAR buffer[buffer_size] = {0};

            int cch = 0;

            if (total_size > 1E8)
            {
               cch = _snwprintf (
                     buffer
                  ,  buffer_size
                  ,  _T ("%s\r\n%.1fG")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E9
                  );
            }
            else if (total_size > 1E5)
            {
               cch = _snwprintf (
                     buffer
                  ,  buffer_size
                  ,  _T ("%s\r\n%.1fM")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E6
                  );
            }
            else if (total_size > 1E2)
            {
               cch = _snwprintf (
                     buffer
                  ,  buffer_size
                  ,  _T ("%s\r\n%.1fk")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E3
                  );
            }
            else
            {
               cch = _snwprintf (
                     buffer
                  ,  buffer_size
                  ,  _T ("%s\r\n%I64d")
                  ,  folder.name.c_str ()
                  ,  total_size
                  );
            }

            FillRect (
                  painter_context.hdc
               ,  &rect
               ,  painter_context.fill_brush
               );

            DrawText (
                  painter_context.hdc
               ,  buffer
               ,  cch
               ,  &rect
               ,  DT_WORD_ELLIPSIS
               );

            rect.right += 1;
            rect.bottom += 1;

            FrameRect (
                  painter_context.hdc
               ,  &rect
               ,  painter_context.frame_brush
               );

         }


         unsigned int proc ()
         {
            HANDLE wait_handles[] =
               {
                     new_frame_request.value.value
                  ,  shutdown_request.value.value
               };

            auto continue_loop   = true   ;
            DWORD wait_result    = 0      ;

            do
            {
               wait_result = WaitForMultipleObjects (
                     2
                  ,  wait_handles
                  ,  false
                  ,  1000);

               
               update_request::ptr request_ptr;

               switch (wait_result)
               {
               case WAIT_OBJECT_0 + 0:
                  while ((request_ptr = update_request_value.reset ()).get ())
                  {
                     if (request_ptr->root)
                     {
                        auto response_ptr = update_response::ptr (
                           new update_response (*request_ptr));

                        // TODO: Should be created from a DC compatible with the window
                        w::device_context bitmap_dc (CreateCompatibleDC (NULL));
                        w::select_object const select_bitmap (
                              bitmap_dc.value
                           ,  response_ptr->bitmap.value
                           );

                        RECT rect   = {0};
                        rect.right  = IMPLICIT_CAST (request_ptr->bitmap_size.x ());
                        rect.bottom = IMPLICIT_CAST (request_ptr->bitmap_size.y ());

                        FillRect (
                              bitmap_dc.value
                           ,  &rect
                           ,  theme::folder_tree::background_brush.value
                           );

                        {
                           w::select_object const select_font (
                                 bitmap_dc.value
                                 ,  theme::default_monospace_font.value
                              );

                           SetBkColor (
                                 bitmap_dc.value
                              ,  theme::folder_tree::background_color);

                           SetTextColor (
                                 bitmap_dc.value
                              ,  theme::folder_tree::folder_background_color);

                           TCHAR buffer [buffer_size * 8] = {0};
                           auto cch = _snwprintf (
                                 buffer
                              ,  buffer_size * 8
                              ,  _T ("Unprocessed folders:%8d  \r\nProcessed folders:%8d  \r\n\r\nTotal file count:%8I64d  \r\nTotal Depth:%8d  ")
                              ,  request_ptr->unprocessed_folder_count
                              ,  request_ptr->processed_folder_count
                              ,  request_ptr->root->get_total_file_count ()
                              ,  request_ptr->root->get_depth ()
                              );

                           DrawText (
                                 bitmap_dc.value
                              ,  buffer
                              ,  cch
                              ,  &rect
                              , DT_RIGHT
                              );
                        }

                        w::select_object const select_font (
                              bitmap_dc.value
                           ,  theme::default_font.value
                           );

                        SetBkColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_background_color);

                        SetTextColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_foreground_color);

                        painter_context const painter_context (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_background_brush.value
                           ,  theme::folder_tree::folder_foreground_brush.value
                           );

                        auto painter_ = [&painter_context] (
                              __int64 const     total_size
                           ,  double const      x
                           ,  double const      y
                           ,  double const      width
                           ,  double const      height
                           ,  f::folder const & folder
                           )
                           {
                              painter (
                                    painter_context
                                 ,  total_size
                                 ,  x
                                 ,  y
                                 ,  width
                                 ,  height
                                 ,  folder
                                 );
                           };

                        auto bitmap_size  = request_ptr->bitmap_size;
                        auto centre       = request_ptr->centre;
                        auto zoom         = request_ptr->zoom;

                        auto current_transform = vt::bitmap_to_screen_transform (
                              vt::transform_direction::forward
                           ,  bitmap_size
                           ,  centre
                           ,  zoom);

                        folder_traverser (
                              request_ptr->bitmap_size
                           ,  current_transform
                           ,  request_ptr->root
                           ,  size_picker
                           ,  painter_
                           );

                        update_response_value.reset (response_ptr.release ());

                        auto result = PostMessage (
                              request_ptr->main_hwnd
                           ,  messages::new_view_available
                           ,  0
                           ,  0);
                        result;
                     }
                  }
                  continue_loop = true;
                  break;
               case WAIT_TIMEOUT:
                  continue_loop = true;
                  break;
               case WAIT_OBJECT_0 + 1:
               case WAIT_ABANDONED:
               default:
                  continue_loop = false;
                  break;
               }
            }
            while (continue_loop);

            return EXIT_SUCCESS;
         }

         w::thread                                    thread            ;
         w::event                                     shutdown_request  ;

      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter::impl
   {
      void do_request (
            folder::folder const * const  root
         ,  HWND const                    main_hwnd
         ,  HDC const                     hdc
         ,  std::size_t const             processed_folder_count
         ,  std::size_t const             unprocessed_folder_count
         ,  RECT const &                  rect   
         ,  coordinate const &            centre
         ,  zoom_factor const &           zoom
         )
      {
         dimension screen_size;
         screen_size.x (rect.right - rect.left);
         screen_size.y (rect.bottom - rect.top);

         auto request = update_request::ptr (
               new update_request (
                     root
                  ,  main_hwnd
                  ,  hdc
                  ,  processed_folder_count
                  ,  unprocessed_folder_count
                  ,  centre
                  ,  zoom
                  ,  screen_size
                  ));

         background_painter.update_request_value.reset (request.release ());
         background_painter.new_frame_request.set ();
      }

      void paint (
            folder::folder const * const  root
         ,  HWND const                    main_hwnd
         ,  HDC const                     hdc
         ,  std::size_t const             processed_folder_count
         ,  std::size_t const             unprocessed_folder_count
         ,  RECT const &                  rect   
         ,  coordinate const &            centre
         ,  zoom_factor const &           zoom
         )
      {
         {
            auto response = background_painter.update_response_value.reset ();
            if (response.get ())
            {
               update_response = response;
            }
         }

         dimension screen_size;
         screen_size.x (rect.right - rect.left);
         screen_size.y (rect.bottom - rect.top);

         if (
               update_response.get ()
            && update_response->centre        == centre
            && update_response->zoom          == zoom
            && update_response->bitmap_size   == screen_size
            )
         {
         }
         else
         {
            do_request (
                  root
               ,  main_hwnd
               ,  hdc
               ,  processed_folder_count
               ,  unprocessed_folder_count
               ,  rect
               ,  centre
               ,  zoom
               );
         }

         if (update_response.get ())
         {
            w::device_context src_dc (CreateCompatibleDC (hdc));
            w::select_object src_select (src_dc.value, update_response->bitmap.value);

            StretchBlt (
                  hdc
               ,  rect.left
               ,  rect.top
               ,  IMPLICIT_CAST (screen_size.x ())
               ,  IMPLICIT_CAST (screen_size.y ())
               ,  src_dc.value
               ,  0
               ,  0
               ,  IMPLICIT_CAST (update_response->bitmap_size.x ())
               ,  IMPLICIT_CAST (update_response->bitmap_size.y ())
               ,  SRCCOPY
               );
         }
         else
         {
            FillRect (
                  hdc
               ,  &rect
               ,  theme::folder_tree::background_brush.value
               );

         }
      }

      background_painter                              background_painter;
      update_response::ptr                            update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter ()
      :  m_impl (new impl())
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::~painter ()
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void painter::do_request (
         folder::folder const * const  root
      ,  HWND const                    main_hwnd
      ,  std::size_t const             processed_folder_count
      ,  std::size_t const             unprocessed_folder_count
      ,  RECT const &                  rect   
      ,  coordinate const &            centre
      ,  zoom_factor const &           zoom
      )
   {
      w::window_device_context window_dc (main_hwnd);
      m_impl->do_request (
            root
         ,  main_hwnd
         ,  window_dc.hdc
         ,  processed_folder_count
         ,  unprocessed_folder_count
         ,  rect   
         ,  centre
         ,  zoom
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void painter::paint (
         folder::folder const * const  root
      ,  HWND const                    main_hwnd
      ,  HDC const                     hdc
      ,  std::size_t const             processed_folder_count
      ,  std::size_t const             unprocessed_folder_count
      ,  RECT const &                  rect   
      ,  coordinate const &            centre
      ,  zoom_factor const &           zoom
      )
   {
      m_impl->paint (
            root
         ,  main_hwnd
         ,  hdc
         ,  processed_folder_count
         ,  unprocessed_folder_count
         ,  rect   
         ,  centre
         ,  zoom
         );
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
