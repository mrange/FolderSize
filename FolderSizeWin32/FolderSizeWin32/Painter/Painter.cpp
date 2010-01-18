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
         int               width             ;
         int               height            ;
         w::device_context compatible_dc     ;
      };

      struct update_response
      {
         transform         transform         ;
         int               width             ;
         int               height            ;
         w::gdi_object     bitmap            ;
      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter::impl
   {
      impl (painter::folder_getter const folder_getter_)
         :  folder_getter     (folder_getter_)
         ,  thread            (create_proc ())
         ,  new_frame_request (true)
         ,  shutdown_request  (false)
      {
      }

      w::thread::proc create_proc () throw ()
      {
         return st::bind (&impl::proc, this);
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

      w::thread_safe_scoped_ptr<update_request>    update_request    ;
      w::thread_safe_scoped_ptr<update_response>   update_response   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   painter::painter (folder_getter const folder_getter)
   {
   }

   void painter::paint (HDC const hdc, transform transform, int width, int height)
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
