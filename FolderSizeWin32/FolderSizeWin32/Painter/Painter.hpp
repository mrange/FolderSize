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
#pragma once
// ----------------------------------------------------------------------------
#include <tchar.h>
#include <windows.h>
// ----------------------------------------------------------------------------
#include <cstddef>
#include <functional>
#include <memory>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../folder.hpp"
#include "../Linear.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------
   typedef linear::vector<double, 2>      coordinate  ;
   typedef linear::vector<double, 2>      zoom_factor ;
   typedef linear::vector<double, 2>      dimension   ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct update_request;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct update_response : boost::noncopyable
   {
      typedef std::auto_ptr<update_response> ptr;

      update_response (
         update_request    const &  update_request_
         );

      coordinate const                 centre            ;
      zoom_factor const                zoom              ;
      dimension const                  bitmap_size       ;
      win32::gdi_object<HBITMAP> const bitmap            ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter : boost::noncopyable
   {
      struct impl;

      painter ();

      update_response::ptr get_bitmap (
            folder::folder const * const  root_
         ,  DWORD const                   main_thread_id_
         ,  coordinate const              centre_        
         ,  zoom_factor const             zoom_          
         ,  dimension const               screen_size_   
         ,  HDC const                     hdc_
         );

      //void paint (
      //      HDC const hdc
      //   ,  coordinate const & centre
      //   ,  zoom_factor const & zoom
      //   ,  dimension const & screen_size);
   private:
      std::auto_ptr<impl> m_impl;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
