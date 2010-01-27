/* ****************************************************************************
 *
 * Copyright (c) Mårten Rånge.
 *
 * This source code is subject to terms and conditions of the Microsoft Public License. A 
 * copy of the license can be found in the License.html file at the root of this distribution. If 
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
            folder_infos & fis
         ,  f::folder const * const f)
      {
         if (f)
         {
            folder_info fi;

            auto folder_count = f->folder_count;

            for (std::size_t iter = 0; iter < folder_count; ++iter)
            {
               fi = fi + update_folder_infos (
                     fis
                  ,  f->sub_folders[iter]);
            }

            fi = fi + folder_info (fi.depth + 1, f->size, f->file_count);

            fis[f] = fi;

            return fi;
         }
         else
         {
            return folder_info ();
         }
      }

      struct update_request : b::noncopyable
      {
         typedef s::auto_ptr<update_request> ptr;

         folder::folder const * const  root              ;

         DWORD const                   main_thread_id    ;
         coordinate const              centre            ;
         zoom_factor const             zoom              ;
         dimension const               screen_size       ;
         w::device_context const       compatible_dc     ;
      };

      struct update_response : b::noncopyable
      {
         typedef s::auto_ptr<update_response> ptr;

         update_response (
            update_request    const &  request
            )
            :  centre      (request.centre)
            ,  zoom        (request.zoom)
            ,  bitmap_size (request.screen_size)
            ,  bitmap      (CreateCompatibleBitmap (
                  request.compatible_dc.value
                , request.screen_size.x ()
                , request.screen_size.y ()))
         {
         }

         coordinate const     centre            ;
         zoom_factor const    zoom              ;
         dimension const      bitmap_size       ;
         w::gdi_object const  bitmap            ;
      };

      struct background_painter
      {
         background_painter ()
            :  thread            (create_proc ())
            ,  new_frame_request (true)
            ,  shutdown_request  (false)
         {
         }

         w::thread_safe_scoped_ptr<update_request>    update_request_value    ;
         w::thread_safe_scoped_ptr<update_response>   update_response_value   ;

      private:
         w::thread::proc create_proc () throw ()
         {
            return st::bind (&background_painter::proc, this);
         }

         template<
               typename TPropertyPickerPredicate
            ,  typename TPainterPredicate>
         void folder_traverser_impl (
               folder_infos const & fis
            ,  double const x
            ,  double const y
            ,  double const x_step_ratio
            ,  double const y_step_ratio
            ,  f::folder const * const folder
            ,  TPropertyPickerPredicate propertyPicker
            ,  TPainterPredicate painter
            )
         {
            if (folder)
            {
               auto fi = fis[folder];

               auto property = propertyPicker (fi);

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
                     auto sfi = fis[sub_folder];
                     auto sproperty = propertyPicker (sfi);
                     auto y_step = y_step_ratio * sproperty;

                     folder_traverser_impl (
                           fis
                        ,  next_x
                        ,  current_y
                        ,  x_step_ratio
                        ,  y_step_ratio
                        ,  sub_folder
                        ,  propertyPicker 
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
         void folder_traverser (
               folder_infos const & fis
            ,  dimension const & size
            ,  f::folder const * const root
            ,  TPropertyPickerPredicate propertyPicker
            ,  TPainterPredicate painter
            )
         {
            if (root && dimension.x > 0 && dimension.y > 0)
            {
               auto fi = fis[root];

               auto property = propertyPicker (fi);

               if (property > 0 && fi.depth > 0)
               {
                  auto x_step_ratio = dimension.x / fi.depth;
                  auto y_step_ratio = dimension.y / property;

                  folder_traverser_impl (
                        fis
                     ,  0.0
                     ,  0.0
                     ,  x_step_ratio
                     ,  y_step_ratio
                     ,  root
                     ,  propertyPicker
                     ,  painter
                     );
               }
            }
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
                     update_request const & request = *request_ptr;

                     if (request.root)
                     {
                        auto response = update_response::ptr (
                           new update_response (request));

                        update_response_value.reset (response.release ());

                        folder_infos fis;

                        auto total_folder_info = update_folder_infos (
                              fis
                           ,  request.root);


                        PostThreadMessage (
                              request.main_thread_id
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
         w::event                                     new_frame_request ;
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

      void paint (
            HDC const hdc
         ,  coordinate const & centre
         ,  zoom_factor const & zoom
         ,  dimension const & screen_size)
      {
         auto response = background_painter.update_response_value.reset ();

         if (response.get ())
         {
            update_response = response;
         }

         if (update_response.get ())
         {
            w::device_context dc (CreateCompatibleDC (hdc));
            w::select_object select_bitmap (dc.value, update_response->bitmap.value);

            if (
                  update_response->centre == centre
               && update_response->zoom == zoom
               && update_response->bitmap_size == screen_size)
            {
               auto xform = make_xform (l::identity<double, 3> ());

               w::set_world_transform world_transform (
                     hdc
                  ,  &xform
                  );
            }
            else
            {
               auto xform = make_xform (l::identity<double, 3> ());

               w::set_world_transform world_transform (
                     hdc
                  ,  &xform
                  );
            }

            BitBlt(
                  hdc
               ,  0
               ,  0
               ,  update_response->bitmap_size.x ()
               ,  update_response->bitmap_size.y ()
               ,  dc.value
               ,  0
               ,  0
               ,  SRCCOPY
               );
         }
      }

      background_painter                              background_painter;
      std::auto_ptr<update_response>                  update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void painter::paint (
         HDC const hdc
      ,  coordinate const & centre
      ,  zoom_factor const & zoom
      ,  dimension const & screen_size)
   {
      m_impl->paint (hdc, centre, zoom, screen_size);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
