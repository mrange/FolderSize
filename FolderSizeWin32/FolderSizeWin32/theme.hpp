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
#include <windows.h>
// ----------------------------------------------------------------------------
#include "win32.hpp"
// ----------------------------------------------------------------------------
namespace theme
{
   extern win32::tstring const               welcome_string                   ;      
   extern win32::tstring const               size_string                   ;      
   extern win32::tstring const               count_string                   ;      

   extern win32::gdi_object<HFONT> const     default_font                     ;
   extern win32::gdi_object<HFONT> const     default_big_font                 ;
   extern win32::gdi_object<HFONT> const     default_monospace_font           ;

   extern COLORREF const                     background_color                 ;
   extern COLORREF const                     background_gradient_top_color    ;
   extern COLORREF const                     background_gradient_bottom_color ;

   extern win32::gdi_object<HBRUSH> const    background_brush                 ;

   extern win32::gdi_object<HBITMAP> const   brand_bitmap                     ;

   namespace folder_tree
   {
      extern win32::tstring const               progress_string               ;
      extern win32::tstring const               info_string                   ;

      extern COLORREF const                     background_color              ;
      extern COLORREF const                     folder_background_color       ;
      extern COLORREF const                     folder_foreground_color       ;

      extern win32::gdi_object<HBRUSH> const    background_brush              ;
      extern win32::gdi_object<HBRUSH> const    folder_background_brush       ;
      extern win32::gdi_object<HBRUSH> const    folder_foreground_brush       ;
   }
}
// ----------------------------------------------------------------------------
