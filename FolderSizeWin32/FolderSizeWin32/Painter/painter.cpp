/* ****************************************************************************
 *
 * Copyright (c) Mårten Rånge.
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
#include <algorithm>
#include <vector>
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

      typedef s::vector<rendered_folder> rendered_folders;

      struct update_request : boost::noncopyable
      {
         typedef std::auto_ptr<update_request> ptr;

         update_request (
               folder::folder const * const  root_
            ,  HWND const                    main_hwnd_
            ,  HDC const                     hdc_
            ,  std::size_t const             processed_folder_count_
            ,  std::size_t const             unprocessed_folder_count_
            ,  select_property::type const   select_property_
            ,  coordinate const &            centre_        
            ,  zoom_factor const &           zoom_          
            ,  dimension const &             screen_size_   
            )
            :  root                       (root_                           )  
            ,  main_hwnd                  (main_hwnd_                      )
            ,  processed_folder_count     (processed_folder_count_         )
            ,  unprocessed_folder_count   (unprocessed_folder_count_       )
            ,  select_property            (select_property_                )
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
         select_property::type const   select_property         ;
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
            :  select_property   (update_request_.select_property    )
            ,  centre            (update_request_.centre             )
            ,  zoom              (update_request_.zoom               )
            ,  bitmap_size       (update_request_.bitmap_size        )
            ,  bitmap            (create_bitmap (
                  update_request_.bitmap_planes
               ,  update_request_.bitmap_bits
               ,  update_request_.bitmap_size               )  )
         {
            rendered_folders.reserve (64);
         }

         select_property::type const      select_property   ;
         coordinate const                 centre            ;
         zoom_factor const                zoom              ;
         dimension const                  bitmap_size       ;
         win32::gdi_object<HBITMAP> const bitmap            ;
         rendered_folders                 rendered_folders  ;
      };

      struct painter_context
      {
         painter_context (
               HDC const hdc_
            )
            :  hdc (hdc_)
         {
         }

         HDC const            hdc               ;
      };

      struct background_painter
      {
         background_painter ()
            :  new_frame_request (w::event_type::auto_reset    )
            ,  shutdown_request  (w::event_type::manual_reset  )
            ,  thread            (_T ("painter"), create_proc ())
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
            ,  rendered_folders &         rendered_folders
            ,  vt::transform const &      transform
            ,  dimension const &          size
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

            if (current_left_top.x () > size.x ())
            {
               return;
            }

            if (current_left_top.y () > size.y ())
            {
               return;
            }

            if (
                  current_right_bottom.x () > 0.0
               && current_right_bottom.y () > 0.0
               && current_left_top.x () < size.x ()
               && current_left_top.y () < size.y ()
               )
            {
               RECT rect         = {0}                      ;
               rect.left         = IMPLICIT_CAST (current_left_top.x ()       );
               rect.top          = IMPLICIT_CAST (current_left_top.y ()       );
               rect.right        = IMPLICIT_CAST (current_right_bottom.x ()   );
               rect.bottom       = IMPLICIT_CAST (current_right_bottom.y ()   );

               view_rect view_rect                                            ;
               view_rect.values[0]      = current_x                           ;
               view_rect.values[1]      = current_y                           ;
               view_rect.values[2]      = current_x + x_step_ratio            ;
               view_rect.values[3]      = current_y + height                  ;

               rendered_folders.push_back (
                  rendered_folder (
                        rect
                     ,  view_rect
                     ,  folder
                     ));

               painter (
                     property
                  ,  rect
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
               auto next_y = current_y + y_step;

               folder_traverser_impl (
                     remaining_levels - 1
                  ,  rendered_folders
                  ,  transform
                  ,  size
                  ,  next_x
                  ,  current_y
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  sub_folder
                  ,  property_picker 
                  ,  painter
                  );

               current_y = next_y;
            }
         }

         template<
               typename TPropertyPickerPredicate
            ,  typename TPainterPredicate>
         static void folder_traverser (
               rendered_folders &         rendered_folders
            ,  dimension const &          size
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
               auto x_step_ratio = 1.0 / depth;
               auto y_step_ratio = 1.0 / property;

               folder_traverser_impl (
                     depth
                  ,  rendered_folders
                  ,  transform
                  ,  size
                  ,  -0.5
                  ,  -0.5
                  ,  x_step_ratio
                  ,  y_step_ratio
                  ,  root
                  ,  property_picker
                  ,  painter
                  );
            }
         }

         static big_size size_picker (
            f::folder const * f
            )
         {
            return f->get_total_size ();
         }

         static big_size physical_size_picker (
            f::folder const * f
            )
         {
            return f->get_total_physical_size ();
         }

         static big_size count_picker (
            f::folder const * f
            )
         {
            return f->get_total_file_count ();
         }

         static void painter (
               painter_context const & painter_context
            ,  big_size const          total_size
            ,  RECT const &            rect
            ,  f::folder const &       folder
            )
         {
            RECT copy_rect = rect;

            TCHAR buffer[buffer_size] = {0};

            int cch = 0;

            if (total_size > 1E8)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fG")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E9
                  );
            }
            else if (total_size > 1E5)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fM")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E6
                  );
            }
            else if (total_size > 1E2)
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%.1fk")
                  ,  folder.name.c_str ()
                  ,  total_size / 1E3
                  );
            }
            else
            {
               cch = _stprintf_s (
                     buffer
                  ,  _T ("%s\r\n%I64d")
                  ,  folder.name.c_str ()
                  ,  total_size
                  );
            }

            COLORREF                      background_color = {0};
            w::gdi_object<HBRUSH> const * background_brush = NULL;

            if (folder.size > folder.physical_size)
            {
               background_color = theme::folder_tree::cfolder_background_color;
               background_brush = &theme::folder_tree::cfolder_background_brush;
            }
            else
            {
               background_color = theme::folder_tree::folder_background_color;
               background_brush = &theme::folder_tree::folder_background_brush;
            }

            SetBkColor (
                  painter_context.hdc
               ,  background_color
               );

            FS_ASSERT (background_brush);

            FillRect (
                  painter_context.hdc
               ,  &copy_rect
               ,  background_brush->value
               );

            DrawText (
                  painter_context.hdc
               ,  buffer
               ,  cch
               ,  &copy_rect
               ,  DT_WORD_ELLIPSIS
               );

            copy_rect.right   += 1;
            copy_rect.bottom  += 1;

            FrameRect (
                  painter_context.hdc
               ,  &copy_rect
               ,  theme::folder_tree::folder_foreground_brush.value
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
                     WIN32_DEBUG_STRING (_T ("Received : New view request"));
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

                        SetBkColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::background_color);

                        SetTextColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_background_color);

                        {
                           w::select_object const select_font (
                                 bitmap_dc.value
                                 ,  theme::default_monospace_font.value
                              );

                           TCHAR buffer [buffer_size * 8] = {0};
                           auto cch = _stprintf_s (
                                 buffer
                              ,  theme::folder_tree::progress_string.c_str ()
                              ,  request_ptr->unprocessed_folder_count
                              ,  request_ptr->processed_folder_count
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

                        if (theme::brand_bitmap.is_valid ())
                        {
                           w::device_context brand_bitmap_dc (CreateCompatibleDC (bitmap_dc.value));

                           w::select_object select_bitmap (
                                 brand_bitmap_dc.value
                                 ,  theme::brand_bitmap.value
                                 );

                           auto brand_bitmap_size = w::get_bitmap_size (
                                 bitmap_dc.value
                              ,  theme::brand_bitmap.value
                              );

                           BitBlt (
                                 bitmap_dc.value
                              ,  rect.right - brand_bitmap_size.cx - 8
                              ,  rect.bottom - brand_bitmap_size.cy - 8
                              ,  brand_bitmap_size.cx
                              ,  brand_bitmap_size.cy
                              ,  brand_bitmap_dc.value
                              ,  0
                              ,  0
                              ,  SRCCOPY
                              );
                        }


                        w::select_object const select_font (
                              bitmap_dc.value
                           ,  theme::default_font.value
                           );

                        DrawText (
                              bitmap_dc.value
                           ,  theme::folder_tree::info_string.c_str ()
                           ,  -1
                           ,  &rect
                           , DT_RIGHT | DT_BOTTOM | DT_SINGLELINE
                           );
                        SetBkColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_background_color);

                        SetTextColor (
                              bitmap_dc.value
                           ,  theme::folder_tree::folder_foreground_color);

                        painter_context const painter_context (
                              bitmap_dc.value
                           );

                        auto painter_ = [&painter_context] (
                              __int64 const     total_size
                           ,  RECT const &      rect
                           ,  f::folder const & folder
                           )
                           {
                              painter (
                                    painter_context
                                 ,  total_size
                                 ,  rect
                                 ,  folder
                                 );
                           };

                        auto bitmap_size  = request_ptr->bitmap_size;
                        auto centre       = request_ptr->centre;
                        auto zoom         = request_ptr->zoom;

                        auto current_transform = vt::view_to_screen (
                              vt::transform_direction::forward
                           ,  bitmap_size
                           ,  centre
                           ,  zoom);

                        auto property_picker = &size_picker;

                        switch (request_ptr->select_property)
                        {
                        case select_property::size:
                           property_picker = &size_picker;
                           break;
                        case select_property::physical_size:
                           property_picker = &physical_size_picker;
                           break;
                        case select_property::count:
                           property_picker = &count_picker;
                           break;
                        default:
                           FS_ASSERT (false);
                           break;
                        }

                        folder_traverser (
                              response_ptr->rendered_folders
                           ,  request_ptr->bitmap_size
                           ,  current_transform
                           ,  request_ptr->root
                           ,  property_picker
                           ,  painter_
                           );

                        update_response_value.reset (response_ptr.release ());

                        WIN32_DEBUG_STRING (_T ("Sending : messages::new_view_available"));

                        auto result = PostMessage (
                              request_ptr->main_hwnd
                           ,  messages::new_view_available
                           ,  0
                           ,  0);
                        UNUSED_VARIABLE (result);
                     }
                  }
                  continue_loop = true;
                  break;
               case WAIT_TIMEOUT:
                  continue_loop = true;
                  break;
               case WAIT_OBJECT_0 + 1:
                  WIN32_DEBUG_STRING (_T ("Received : Terminate painter"));
                  continue_loop = false;
                  break;
               case WAIT_ABANDONED:
                  WIN32_DEBUG_STRING (_T ("Received : Abandon painter"));
                  continue_loop = false;
                  break;
               default:
                  {
                  TCHAR buffer[buffer_size] = {0};
                  _stprintf_s (
                        buffer
                     ,  _T ("Received : Unknown request : %X (@err = %X)")
                     ,  wait_result
                     ,  GetLastError ()
                     );
                  w::trace_string (buffer);
                  continue_loop = false;
                  }
                  break;
               }
            }
            while (continue_loop);

            return EXIT_SUCCESS;
         }

         w::event                                     shutdown_request  ;
         w::thread                                    thread            ;

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
         ,  select_property::type         select_property
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
                  ,  select_property
                  ,  centre
                  ,  zoom
                  ,  screen_size
                  ));

         background_painter.update_request_value.reset (request.release ());
         WIN32_DEBUG_STRING (_T ("Sending : New view request"));
         background_painter.new_frame_request.set ();
      }

      void paint (
            folder::folder const * const  root
         ,  HWND const                    main_hwnd
         ,  HDC const                     hdc
         ,  std::size_t const             processed_folder_count
         ,  std::size_t const             unprocessed_folder_count
         ,  select_property::type         select_property
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
            && update_response->select_property == select_property
            && update_response->centre          == centre
            && update_response->zoom            == zoom
            && update_response->bitmap_size     == screen_size
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
               ,  select_property
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

      rendered_folder const hit_test (
            POINT const & offset
         )
      {
         if (!update_response.get ())
         {
            return rendered_folder ();
         }

         rendered_folders const & current_rendered_folders = update_response->rendered_folders;

         auto end    = current_rendered_folders.end ();
         auto find   = s::find_if (
               current_rendered_folders.begin ()
            ,  end
            ,  [offset] (rendered_folder const & rf)
               {
                  return w::is_inside (rf.render_rect, offset);
               });

         if (find == end)
         {
            return rendered_folder ();
         }

         return *find;
      }

      background_painter                              background_painter;
      update_response::ptr                            update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   rendered_folder::rendered_folder ()
      :  render_rect (w::zero_rect ()  )
      ,  folder      (NULL             )
   {
   }

   rendered_folder::rendered_folder (
         RECT const &                  render_rect_
      ,  ::painter::view_rect const  & view_rect_
      ,  f::folder const * const       folder_
      )
      :  render_rect (render_rect_  )
      ,  view_rect   (view_rect_    )
      ,  folder      (folder_       )
   {
   }

   rendered_folder::rendered_folder (rendered_folder const && rf)
      :  render_rect (rf.render_rect   )
      ,  view_rect   (rf.view_rect     )
      ,  folder      (rf.folder        )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter ()
      :  m_impl (new impl ())
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
      ,  select_property::type         select_property
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
         ,  select_property
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
      ,  select_property::type         select_property
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
         ,  select_property
         ,  rect   
         ,  centre
         ,  zoom
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   rendered_folder const painter::hit_test (
         POINT const & offset
      )
   {
      return m_impl->hit_test (
            offset
         );
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
