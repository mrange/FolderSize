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
#include "Painter.hpp"
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
      typedef linear::matrix<double, 3, 3>      transform   ;

      struct update_request : b::noncopyable
      {
         typedef s::auto_ptr<update_request> ptr;

         folder::folder const    root              ;

         DWORD const             main_thread_id    ;
         coordinate const        centre            ;
         zoom_factor const       zoom              ;
         dimension const         screen_size       ;
         w::device_context const compatible_dc     ;
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

               
               switch (wait_result)
               {
               case WAIT_OBJECT_0 + 0:
                  {
                     auto request_ptr = update_request_value.reset ();

                     if (request_ptr.get ())
                     {
                        update_request const & request = *request_ptr;
                        auto response = update_response::ptr (
                           new update_response (request));

                        update_response_value.reset (response.release ());

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
