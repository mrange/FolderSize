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
   extern win32::gdi_object<HFONT> const default_font                ;
   extern win32::gdi_object<HFONT> const default_big_font            ;
   extern win32::gdi_object<HFONT> const default_monospace_font      ;

   namespace folder_tree
   {
      extern COLORREF const background_color                         ;
      extern COLORREF const folder_background_color                  ;
      extern COLORREF const folder_foreground_color                  ;

      extern win32::gdi_object<HBRUSH> const background_brush        ;
      extern win32::gdi_object<HBRUSH> const folder_background_brush ;
      extern win32::gdi_object<HBRUSH> const folder_foreground_brush ;
   }
}
// ----------------------------------------------------------------------------
