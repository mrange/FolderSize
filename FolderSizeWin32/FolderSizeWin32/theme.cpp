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
#include "StdAfx.h"
// ----------------------------------------------------------------------------
#include "theme.hpp"
// ----------------------------------------------------------------------------
namespace theme
{
   namespace w = win32;

   win32::gdi_object<HFONT> const default_font                 =  w::create_standard_message_font  (                       );
   win32::gdi_object<HFONT> const default_big_font             =  w::create_standard_message_font  (48                     );
   win32::gdi_object<HFONT> const default_monospace_font       =  w::create_font                   (_T("Courier New"), 18  );


   namespace folder_tree
   {
      COLORREF const background_color                          = RGB (0x29, 0x39, 0x55);
      COLORREF const folder_background_color                   = RGB (0xBC, 0xC7, 0xD8);
      COLORREF const folder_foreground_color                   = RGB (0x42, 0x48, 0x51);

      win32::gdi_object<HBRUSH> const background_brush         (CreateSolidBrush (background_color          ));
      win32::gdi_object<HBRUSH> const folder_background_brush  (CreateSolidBrush (folder_background_color   ));
      win32::gdi_object<HBRUSH> const folder_foreground_brush  (CreateSolidBrush (folder_foreground_color   ));
   }
}
// ----------------------------------------------------------------------------
