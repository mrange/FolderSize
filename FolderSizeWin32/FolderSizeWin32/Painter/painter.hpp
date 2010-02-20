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
#include <memory>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../folder.hpp"
#include "../linear.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------
   typedef linear::vector<double, 2>      coordinate  ;
   typedef linear::vector<double, 2>      zoom_factor ;
   typedef linear::vector<double, 2>      dimension   ;
   typedef linear::vector<double, 4>      view_rect   ;
   namespace select_property
   {
      enum type
      {
         size  ,
         count ,
      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct rendered_folder
   {
      rendered_folder ();
      rendered_folder (
            RECT const &                  render_rect
         ,  view_rect const  &            view_rect
         ,  folder::folder const * const  folder
         );

      rendered_folder (rendered_folder const &&);

      RECT const                    render_rect ;
      view_rect const               view_rect   ;
      folder::folder const * const  folder      ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct painter : boost::noncopyable
   {
      painter ();
      ~painter () throw ();

      void do_request (
            folder::folder const * const  root
         ,  HWND const                    main_hwnd
         ,  std::size_t const             processed_folder_count
         ,  std::size_t const             unprocessed_folder_count
         ,  select_property::type         select_property
         ,  RECT const &                  rect   
         ,  coordinate const &            centre
         ,  zoom_factor const &           zoom
         );

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
         );

      rendered_folder const hit_test (
            POINT const & offset
         );

   private:
      struct impl;

      std::auto_ptr<impl> m_impl;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
