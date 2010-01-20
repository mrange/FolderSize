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
      struct update_request
      {
         transform         transform         ;
         std::size_t       width             ;
         std::size_t       height            ;
         w::device_context compatible_dc     ;
      };

      struct update_response
      {
         transform         transform         ;
         std::size_t       width             ;
         std::size_t       height            ;
         w::gdi_object     bitmap            ;
      };

      struct background_painter
      {
         background_painter (painter::folder_getter const folder_getter_)
            :  folder_getter     (folder_getter_)
            ,  thread            (create_proc ())
            ,  new_frame_request (true)
            ,  shutdown_request  (false)
         {
         }

         w::thread_safe_scoped_ptr<update_request>    update_request    ;
         w::thread_safe_scoped_ptr<update_response>   update_response   ;

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
                     auto request = update_request.reset ();
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

         painter::folder_getter                       folder_getter     ;
         w::thread                                    thread            ;
         w::event                                     new_frame_request ;
         w::event                                     shutdown_request  ;

      };

      XFORM const make_xform (transform const & transform)
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

      XFORM const make_xform (
         std::auto_ptr<update_response> const update_response,
         transform const & transform,
         std::size_t const height,
         std::size_t const width)
      {
         XFORM form = {0};

         return form;
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter::impl
   {

      void paint (
            HDC const hdc
         ,  transform const & transform
         ,  std::size_t const width
         ,  std::size_t const height)
      {
         auto response = background_painter.update_response.reset ();

         if (response.get ())
         {
            update_response = response;
         }

         if (update_response.get ())
         {
            w::device_context dc (CreateCompatibleDC (hdc));
            w::select_object select_bitmap (dc.value, update_response->bitmap.value);

            if (
                  update_response->transform == transform
               && update_response->height == height
               && update_response->width == width)
            {
            }
            else
            {
            }

            XFORM const form = make_xform (
               update_response,
               transform,
               height,
               width);

            w::set_world_transform world_transform (
               hdc,
               &form
               );

            BitBlt(
               hdc,
               0,
               0,
               width,
               height,
               dc.value,
               0,
               0,
               SRCCOPY);
         }
      }

      background_painter                              background_painter;
      std::auto_ptr<update_response>                  update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter (folder_getter const folder_getter)
   {
   }

   void painter::paint (
            HDC const hdc
         ,  transform const & transform
         ,  std::size_t const width
         ,  std::size_t const height)
   {
      m_impl->paint (hdc, transform, width, height);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
