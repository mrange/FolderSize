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
#include "resource.h"
// ----------------------------------------------------------------------------
namespace theme
{
   namespace w = win32;  

   w::tstring const           welcome_string                   = w::load_string_resource  (IDC_WELCOME_MSG            , _T("Click Go to start...") );      
   w::tstring const           size_string                      = w::load_string_resource  (IDC_SIZE_CHOICE            , _T("Size")                 );            
   w::tstring const           physical_size_string             = w::load_string_resource  (IDC_PHYSICAL_SIZE_CHOICE   , _T("Physical")             );            
   w::tstring const           count_string                     = w::load_string_resource  (IDC_COUNT_CHOICE           , _T("Count")                );            

   w::gdi_object<HFONT> const default_font                     = w::create_standard_font  (w::standard_font::caption  , 20 );
   w::gdi_object<HFONT> const default_big_font                 = w::create_standard_font  (w::standard_font::caption  , 48  );
   w::gdi_object<HFONT> const default_monospace_font           = w::create_font           (_T ("Courier New")         , 18  );

   COLORREF const background_color                             = RGB (0xDE, 0xDE, 0xDE);
   COLORREF const background_gradient_top_color                = RGB (0xF0, 0xF0, 0xF0);
   COLORREF const background_gradient_bottom_color             = RGB (0xCC, 0xCC, 0xCC);

   w::gdi_object<HBRUSH> const background_brush                (CreateSolidBrush (background_color)                        );

   w::gdi_object<HBITMAP> const   brand_bitmap                 (w::load_bitmap_resource (IDB_BRAND_BITMAP)             );

   namespace folder_tree
   {
      w::tstring const        progress_string                  = w::load_string_resource (IDC_FOLDERTREE_PROGRESS , _T ("Unprocessed folders:%8d  \r\nProcessed folders:%8d  \r\n\r\nMax folder depth:%8d  "));            
      w::tstring const        info_string                      = w::load_string_resource (IDC_FOLDERTREE_INFO     , _T ("Use mouse wheel to zoom, left button to move, right button resets  "));            

      COLORREF const background_color                          = RGB (0x29, 0x39, 0x55);
      COLORREF const folder_background_color                   = RGB (0xBC, 0xC7, 0xD8);
      COLORREF const cfolder_background_color                  = RGB (0xD8, 0xC6, 0xBC);
      COLORREF const rfolder_background_color                  = RGB (0xC3, 0xD3, 0xBA);
      COLORREF const folder_foreground_color                   = RGB (0x42, 0x48, 0x51);

      w::gdi_object<HBRUSH> const background_brush             (CreateSolidBrush (background_color          ));
      w::gdi_object<HBRUSH> const folder_background_brush      (CreateSolidBrush (folder_background_color   ));
      w::gdi_object<HBRUSH> const cfolder_background_brush     (CreateSolidBrush (cfolder_background_color  ));
      w::gdi_object<HBRUSH> const rfolder_background_brush     (CreateSolidBrush (rfolder_background_color  ));
      w::gdi_object<HBRUSH> const folder_foreground_brush      (CreateSolidBrush (folder_foreground_color   ));
   }
}
// ----------------------------------------------------------------------------
