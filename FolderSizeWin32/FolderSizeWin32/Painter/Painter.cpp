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
#include "Painter.hpp"
// ----------------------------------------------------------------------------
#include <tuple>
#include <unordered_map>
// ----------------------------------------------------------------------------
#include "../Linear.hpp"
#include "../Messages.hpp"
#include "../Win32.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace b    = boost     ;
   namespace f    = folder    ;
   namespace l    = linear    ;
   namespace s    = std       ;
   namespace st   = std::tr1  ;
   namespace w    = win32     ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct update_request : boost::noncopyable
   {
      typedef std::auto_ptr<update_request> ptr;

      update_request (
            folder::folder const * const  root_
         ,  DWORD const                   main_thread_id_
         ,  coordinate const              centre_        
         ,  zoom_factor const             zoom_          
         ,  dimension const               screen_size_   
         ,  HDC const                     hdc_
         );

      folder::folder const * const  root              ;

      DWORD const                   main_thread_id    ;
      coordinate const              centre            ;
      zoom_factor const             zoom              ;
      dimension const               screen_size       ;
      win32::device_context const   compatible_dc     ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      const double cut_off_y = 10.0;

      typedef linear::matrix<double, 3, 3>                  transform         ;

      struct folder_info
      {
         folder_info ()
            :  depth (1)
            ,  size  (0)
            ,  count (0)
         {
         }

         folder_info (
               std::size_t depth_
            ,  __int64     size_
            ,  __int64     count_)
            :  depth (depth_)
            ,  size  (size_)
            ,  count (count_)
         {
         }

         std::size_t    depth    ;
         __int64        size   ;
         __int64        count  ;

      };

      static folder_info const operator+ (
            folder_info const & left
         ,  folder_info const & right)
      {
         return folder_info (
               std::max (left.depth, right.depth)
            ,   left.size + right.size
            ,  left.count + right.count);
      }

      typedef st::unordered_map<f::folder const *, folder_info> folder_infos  ;

      folder_info const update_folder_infos (
            folder_infos & folder_infos
         ,  f::folder const * const folder)
      {
         if (folder)
         {
            folder_info fi;

            auto folder_count = folder->folder_count;

            for (std::size_t iter = 0; iter < folder_count; ++iter)
            {
               fi = fi + update_folder_infos (
                     folder_infos
                  ,  folder->sub_folders[iter]);
            }

            fi = fi + folder_info (fi.depth + 1, folder->size, folder->file_count);

            folder_infos[folder] = fi;

            return fi;
         }
         else
         {
            return folder_info ();
         }
      }

      struct painter_context
      {
         painter_context (HDC hdc_, HBRUSH fill_brush_)
            :  hdc(hdc_)
            ,  fill_brush(fill_brush_)
         {
         }

         HDC const            hdc               ;
         HBRUSH const         fill_brush        ;
      };

      struct background_painter
      {
         background_painter ()
            :  thread            (create_proc ())
            ,  new_frame_request (true)
            ,  shutdown_request  (false)
         {
         }

         ~background_painter () throw ()
         {
            shutdown_request.set ();
            thread.join (1000);
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
               folder_infos const & folder_infos
            ,  double const x
            ,  double const y
            ,  double const x_step_ratio
            ,  double const y_step_ratio
            ,  f::folder const * const folder
            ,  TPropertyPickerPredicate property_picker
            ,  TPainterPredicate painter
            )
         {
            if (folder)
            {
               auto folder_info = folder_infos.at (folder);

               auto property = property_picker (folder_info);

               auto current_x = x;
               auto current_y = y;
               auto next_x = current_x + x_step_ratio;

               painter (
                     property
                  ,  current_x
                  ,  current_y
                  ,  x_step_ratio
                  ,  property * y_step_ratio
                  ,  *folder
                  );
                  
               auto folder_count = folder->folder_count;

               for (std::size_t iter = 0; iter < folder_count; ++iter)
               {
                  auto sub_folder = folder->sub_folders[iter];

                  if (sub_folder)
                  {
                     auto sub_folder_info = folder_infos.at (sub_folder);
                     auto sub_property = property_picker (sub_folder_info);
                     auto y_step = y_step_ratio * sub_property;

                     folder_traverser_impl (
                           folder_infos
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
            }
         }

         template<
               typename TPropertyPickerPredicate
            ,  typename TPainterPredicate>
         static void folder_traverser (
               folder_infos const & folder_infos
            ,  dimension const & size
            ,  f::folder const * const root
            ,  TPropertyPickerPredicate property_picker
            ,  TPainterPredicate painter
            )
         {
            if (root && size.x ()> 0 && size.y () > 0)
            {
               auto folder_info = folder_infos.at(root);

               auto property = property_picker (folder_info);

               if (property > 0 && folder_info.depth > 0)
               {
                  auto x_step_ratio = size.x ()/ folder_info.depth;
                  auto y_step_ratio = size.y ()/ property;

                  folder_traverser_impl (
                        folder_infos
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
         }

         static __int64 size_picker(
            folder_info const & folder_info
            )
         {
            return folder_info.size;
         }

         static void painter(
               painter_context const & painter_context
            ,  __int64 total_size
            ,  double x
            ,  double y
            ,  double width
            ,  double height
            ,  f::folder const & folder
            )
         {
            RECT rect         = {0}          ;
            rect.left         = x            ;
            rect.top          = y            ;
            rect.right        = x + width    ;
            rect.bottom       = y + height   ;

            FillRect (
                  painter_context.hdc
               ,  &rect
               ,  painter_context.fill_brush
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

               
               std::auto_ptr<update_request> request_ptr;

               switch (wait_result)
               {
               case WAIT_OBJECT_0 + 0:
                  while ((request_ptr = update_request_value.reset ()).get ())
                  {
                     if (request_ptr->root)
                     {
                        auto response_ptr = update_response::ptr (
                           new update_response (*request_ptr));

                        folder_infos folder_infos;

                        auto total_folder_info = update_folder_infos (
                              folder_infos
                           ,  request_ptr->root);
                        
                        int i = GetDeviceCaps (request_ptr->compatible_dc.value, BITSPIXEL);

                        w::select_object const select_bitmap (
                              request_ptr->compatible_dc.value
                           ,  response_ptr->bitmap.value);

                        w::gdi_object<HBRUSH> const black_brush (CreateSolidBrush (RGB(0x00, 0x00, 0xFF)));
                        w::gdi_object<HBRUSH> const white_brush (CreateSolidBrush (RGB(0xFF, 0x00, 0x00)));

                        painter_context const painter_context (
                              request_ptr->compatible_dc.value
                           ,  black_brush.value
                           );

                        RECT rect = {0};
                        rect.right = request_ptr->screen_size.x ();
                        rect.bottom = request_ptr->screen_size.y ();
                        FillRect (
                              request_ptr->compatible_dc.value
                           ,  &rect
                           ,  white_brush.value);

                        auto painter_ = [&painter_context] (
                              __int64 total_size
                           ,  double x
                           ,  double y
                           ,  double width
                           ,  double height
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
                                 ,  folder);
                           };

                        //folder_traverser (
                        //      folder_infos
                        //   ,  request_ptr->screen_size
                        //   ,  request_ptr->root
                        //   ,  size_picker
                        //   ,  painter_);

                        update_response_value.reset (response_ptr.release ());

                        auto result = PostThreadMessage (
                              request_ptr->main_thread_id
                           ,  messages::refresh_view
                           ,  0
                           ,  0);
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

      XFORM const make_xform (
         transform const & transform)
      {
         XFORM form = {0};

         form.eM11   = transform(0,0);
         form.eM12   = transform(0,1);
         form.eDx    = transform(0,2);

         form.eM21   = transform(1,0);
         form.eM22   = transform(1,1);
         form.eDy    = transform(1,2);

         return form;
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter::impl
   {

      //void paint (
      //      HDC const hdc
      //   ,  coordinate const & centre
      //   ,  zoom_factor const & zoom
      //   ,  dimension const & screen_size)
      //{
      //   auto response = background_painter.update_response_value.reset ();

      //   if (response.get ())
      //   {
      //      update_response = response;
      //   }

      //   if (update_response.get ())
      //   {
      //      w::device_context dc (CreateCompatibleDC (hdc));
      //      w::select_object select_bitmap (dc.value, update_response->bitmap.value);

      //      if (
      //            update_response->centre == centre
      //         && update_response->zoom == zoom
      //         && update_response->bitmap_size == screen_size)
      //      {
      //         auto xform = make_xform (l::identity<double, 3> ());

      //         w::set_world_transform world_transform (
      //               hdc
      //            ,  &xform
      //            );
      //      }
      //      else
      //      {
      //         auto xform = make_xform (l::identity<double, 3> ());

      //         w::set_world_transform world_transform (
      //               hdc
      //            ,  &xform
      //            );
      //      }

      //      BitBlt(
      //            hdc
      //         ,  0
      //         ,  0
      //         ,  update_response->bitmap_size.x ()
      //         ,  update_response->bitmap_size.y ()
      //         ,  dc.value
      //         ,  0
      //         ,  0
      //         ,  SRCCOPY
      //         );
      //   }
      //}
      //std::auto_ptr<update_response>                  update_response   ;

      update_response::ptr get_bitmap (
            update_request::ptr & request_)
      {
         auto request = request_;
         auto response = background_painter.update_response_value.reset ();

         if (request.get ())
         {
            if (
                  response.get ()
               && response->centre        == request->centre
               && response->zoom          == request->zoom
               && response->bitmap_size   == request->screen_size)
            {
            }
            else
            {
               background_painter.update_request_value.reset (request.release ());   
               background_painter.new_frame_request.set ();
            }
         }

         return response;
      }

      background_painter                              background_painter;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   update_request::update_request (
         folder::folder const * const  root_
      ,  DWORD const                   main_thread_id_
      ,  coordinate const              centre_        
      ,  zoom_factor const             zoom_          
      ,  dimension const               screen_size_   
      ,  HDC const                     hdc_
      )
      :  root           (root_                     )
      ,  main_thread_id (main_thread_id_           )
      ,  centre         (centre_                   )
      ,  zoom           (zoom_                     )
      ,  screen_size    (screen_size_              )
      ,  compatible_dc  (CreateCompatibleDC (hdc_) )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   update_response::update_response (
      update_request    const &  update_request_
      )
      :  centre      (update_request_.centre       )
      ,  zoom        (update_request_.zoom         )
      ,  bitmap_size (update_request_.screen_size  )
      ,  bitmap      (CreateCompatibleBitmap (
            update_request_.compatible_dc.value
          , update_request_.screen_size.x ()
          , update_request_.screen_size.y ()))
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   //void painter::paint (
   //      HDC const hdc
   //   ,  coordinate const & centre
   //   ,  zoom_factor const & zoom
   //   ,  dimension const & screen_size)
   //{
   //   m_impl->paint (hdc, centre, zoom, screen_size);
   //}
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter ()
      :  m_impl (new impl())
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   update_response::ptr painter::get_bitmap (
            folder::folder const * const  root_
         ,  DWORD const                   main_thread_id_
         ,  coordinate const              centre_        
         ,  zoom_factor const             zoom_          
         ,  dimension const               screen_size_   
         ,  HDC const                     hdc_
         )
   {
      update_request::ptr request (new update_request (
            root_      
         ,  main_thread_id_
         ,  centre_        
         ,  zoom_          
         ,  screen_size_   
         ,  hdc_));

      return m_impl->get_bitmap (request);
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
