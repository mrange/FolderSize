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

   w::tstring const           welcome_string                      = w::load_string_resource  (IDC_WELCOME_MSG            , _T("Click Go to start...") );      
   w::tstring const           size_string                         = w::load_string_resource  (IDC_SIZE_CHOICE            , _T("Size")                 );            
   w::tstring const           physical_size_string                = w::load_string_resource  (IDC_PHYSICAL_SIZE_CHOICE   , _T("Physical")             );            
   w::tstring const           count_string                        = w::load_string_resource  (IDC_COUNT_CHOICE           , _T("Count")                );            

   w::gdi_object<HFONT> const default_font                        = w::create_standard_font  (w::standard_font::caption  , 20 );
   w::gdi_object<HFONT> const default_big_font                    = w::create_standard_font  (w::standard_font::caption  , 48  );
   w::gdi_object<HFONT> const default_monospace_font              = w::create_font           (_T ("Courier New")         , 18  );

   COLORREF const background_color                                = RGB (0xDE, 0xDE, 0xDE);
   COLORREF const background_gradient_top_color                   = RGB (0xF0, 0xF0, 0xF0);
   COLORREF const background_gradient_bottom_color                = RGB (0xCC, 0xCC, 0xCC);

   w::gdi_object<HBRUSH> const background_brush                   (CreateSolidBrush (background_color)                        );

   w::gdi_object<HBITMAP> const   brand_bitmap                    (w::load_bitmap_resource (IDB_BRAND_BITMAP)             );

   namespace folder_tree
   {
      w::tstring const        progress_string                     = w::load_string_resource (IDC_FOLDERTREE_PROGRESS , _T ("Unprocessed folders:%8d  \r\nProcessed folders:%8d  \r\n\r\nMax folder depth:%8d  "));            
      w::tstring const        info_string                         = w::load_string_resource (IDC_FOLDERTREE_INFO     , _T ("Use mouse wheel to zoom, left button to move, right button resets  "));            
      w::tstring const        merged_folder_string                = w::load_string_resource (IDC_FOLDERTREE_MERGED   , _T ("Many folders"));            

      COLORREF const          foreground_color                    = RGB (0xBC, 0xC7, 0xD8);
      COLORREF const          background_color                    = RGB (0x29, 0x39, 0x55);
      COLORREF const          folder_background_color[6]          =
         {
            RGB (0xF5, 0x3C, 0x06)  ,
            RGB (0xE9, 0x58, 0x30)  ,
            RGB (0xDD, 0x74, 0x5B)  ,
            RGB (0xD2, 0x91, 0x86)  ,
            RGB (0xC6, 0xAD, 0xB1)  ,
            RGB (0xBC, 0xC7, 0xD8)  ,
         };
      COLORREF const          merged_folder_background_color      = RGB (0xC3, 0xD3, 0xBA);
      COLORREF const          folder_foreground_color             = RGB (0x42, 0x48, 0x51);

      w::gdi_object<HBRUSH> const   foreground_brush              (CreateSolidBrush (foreground_color                ));
      w::gdi_object<HBRUSH> const   background_brush              (CreateSolidBrush (background_color                ));
      w::gdi_object<HBRUSH> const   folder_background_brush[6]    =
         {
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[0]))   ,
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[1]))   ,
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[2]))   ,
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[3]))   ,
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[4]))   ,
            w::gdi_object<HBRUSH>                                 (CreateSolidBrush (folder_background_color[5]))   ,
         };
      w::gdi_object<HBRUSH> const   merged_folder_background_brush(CreateSolidBrush (merged_folder_background_color  ));
      w::gdi_object<HBRUSH> const   folder_foreground_brush       (CreateSolidBrush (folder_foreground_color         ));

      color_legend color_legends [7]  =
         {
            {folder_background_brush[0].value      ,  _T ("Activity the last hour") },
            {folder_background_brush[1].value      ,  _T ("Last day")               },
            {folder_background_brush[2].value      ,  _T ("Last 7 days")            },
            {folder_background_brush[3].value      ,  _T ("Last 31 days")           },
            {folder_background_brush[4].value      ,  _T ("Last 365 days")          },
            {folder_background_brush[5].value      ,  _T ("No activity")            },
            {merged_folder_background_brush.value  ,  _T ("Many folders")           },
         };


   }
}
// ----------------------------------------------------------------------------
