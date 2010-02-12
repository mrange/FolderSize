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
#include "../resource.h"
// ----------------------------------------------------------------------------
#include <windows.h>
#include <commctrl.h>
// ----------------------------------------------------------------------------
#include <functional>
// ----------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../Painter/painter.hpp"
#include "../Traverser/traverser.hpp"
#include "../messages.hpp"
#include "../utility.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#pragma comment(lib, "comctl32.lib")
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
namespace main_window
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace s    = std          ;
   namespace st   = s::tr1       ;
   namespace b    = boost        ;
   namespace t    = traverser    ;
   namespace p    = painter      ;
   namespace w    = win32        ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      //struct MARGINS
      //{
      //   int cxLeftWidth      ;
      //   int cxRightWidth     ;
      //   int cyTopHeight      ;
      //   int cyBottomHeight   ;
      //};
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      //typedef HRESULT (WINAPI *DwmExtendFrameIntoClientAreaPtr)(HWND, const MARGINS*);
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      linear::vector<double, 2> const create_vector (double x, double y)
      {
         linear::vector<double, 2> vector;
         vector.x (x);
         vector.y (y);
         return vector;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      namespace window_type
      {
         enum type
         {
            nowindow = 0   ,
            static_        ,
            button         ,
            edit           ,
         };
      }

      struct child_window
      {
         int                  id;
         int                  left;
         int                  top;
         int                  right;
         int                  bottom;
         window_type::type    window_type;
         DWORD                style;
         DWORD                extended_style;
         LPCTSTR              title;

         HWND                 hwnd;
      };
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      struct state
      {
         typedef s::auto_ptr<state> ptr               ;
         t::traverser               traverser         ;
         p::painter                 painter           ;

         state (
               HWND const           main_hwnd
            ,  w::tstring const &   path
            )
            :  traverser      (main_hwnd, path)
         {
         }
      };
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      const int            s_max_load_string                      = 100;
      HINSTANCE            s_instance                             = NULL;
      TCHAR                s_title           [s_max_load_string]  = {0};
      TCHAR                s_window_class    [s_max_load_string]  = {0};
      child_window         s_child_window    []                   =
      {
         {  IDM_GO_PAUSE   , 8      , 8   , 8 + 80    , 8 + 32 , window_type::button  , 0    ,  0  , _T("Go")           },
         {  IDM_STOP       , 100    , 8   , 100 + 80  , 8 + 32 , window_type::button  , 0    ,  0  , _T("Stop")         },
         {  IDM_PATH       , 200    , 8   , -8        , 8 + 32 , window_type::edit    , 0    ,  0  , _T("C:\\Windows")  },
         {  IDM_FOLDERTREE , 8      , 48  , -8        , -8     , window_type::nowindow, 0    ,  0  , _T("")             },
         {0},
      };

      child_window &       s_folder_tree                          = s_child_window[3];

      state::ptr           s_state;

      //w::dll               s_dwm_dll (_T ("DwmApi"));
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      //w::function_pointer<DwmExtendFrameIntoClientAreaPtr> DwmExtendFrameIntoClientArea (
      //      s_dwm_dll.value
      //   ,  "DwmExtendFrameIntoClientArea");
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      typedef st::function<void (child_window&)>  child_window_predicate;
      void for_all_child_windows (child_window_predicate const predicate)
      {
         for (int iter = 0; true; ++iter)
         {
            child_window & wc = s_child_window[iter];

            if (wc.window_type == window_type::nowindow)
            {
               return;
            }

            predicate (wc);
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      SIZE const get_client_size (HWND const hwnd)
      {
         RECT rect = {0};
         auto result = GetClientRect (hwnd, &rect);
         result;

         BOOST_ASSERT (result);
         
         SIZE sz = {0};
         sz.cx = rect.right - rect.left;
         sz.cy = rect.bottom - rect.top;

         return sz;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      RECT const calculate_window_coordinate (
            SIZE const & main_window_size
         ,  child_window const & child_window)
      {
         auto calculate_coord = [] (int c, int rc) -> int
         {
            if (c < 0)
            {
               return rc + c;
            }
            else
            {
               return c;
            }
         };

         auto real_left    = calculate_coord (child_window.left    , main_window_size.cx);
         auto real_top     = calculate_coord (child_window.top     , main_window_size.cy);
         auto real_right   = calculate_coord (child_window.right   , main_window_size.cx);
         auto real_bottom  = calculate_coord (child_window.bottom  , main_window_size.cy);

         RECT result = {0};
         result.left    = real_left    ;
         result.top     = real_top     ;
         result.right   = real_right   ;
         result.bottom  = real_bottom  ;

         return result;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      RECT const calculate_window_coordinate (
            HWND const hwnd
         ,  child_window const & child_window
         )
      {
         auto sz = get_client_size (hwnd);

         return calculate_window_coordinate (sz, child_window);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void update_child_window_positions (HWND const hwnd)
      {
         SIZE sz = get_client_size (hwnd);

         for_all_child_windows (
            [&sz] (child_window & wc)
            {
               HWND child_window = wc.hwnd;
               
               if (wc.hwnd && wc.window_type != window_type::nowindow)
               {
                  auto rect = calculate_window_coordinate (
                        sz
                     ,  wc
                     );

                  SetWindowPos (
                        child_window
                     ,  NULL
                     ,  rect.left
                     ,  rect.top
                     ,  rect.right     - rect.left
                     ,  rect.bottom    - rect.top
                     ,  SWP_NOACTIVATE | SWP_NOZORDER );
               }
            });
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void invalidate_folder_tree_area (HWND const hwnd)
      {
         auto rect = calculate_window_coordinate (
               hwnd
            ,  s_folder_tree
            );
         UNUSED_VARIABLE (rect);

         auto result = InvalidateRect (
               hwnd
            ,  &rect
            , FALSE
            );
         UNUSED_VARIABLE (result);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      LRESULT CALLBACK window_process (
            HWND const     hwnd
         ,  UINT const     message
         ,  WPARAM const   wParam
         ,  LPARAM const   lParam
         )
      {
         int wmId       = {0};
         int wmEvent    = {0};
         PAINTSTRUCT ps = {0};
         HDC hdc        = {0};

         switch (message)
         {
         case messages::new_view_available:
            invalidate_folder_tree_area (hwnd);
            break;
         case messages::folder_structure_changed:
            if (s_state.get ())
            {
               auto rect = calculate_window_coordinate (
                     hwnd
                  ,  s_folder_tree
                  );

               s_state->painter.do_request (
                     s_state->traverser.get_root ()
                  ,  hwnd
                  ,  rect
                  ,  create_vector (0.0, 0.0)
                  ,  create_vector (1.0, 1.0)
                  );
            }
            break;
         case WM_COMMAND:
            wmId    = LOWORD (wParam);
            wmEvent = HIWORD (wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            //case IDM_ABOUT:
            //   DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUTBOX), hwnd, About);
            //   break;
            //case IDM_EXIT:
            //   DestroyWindow (hwnd);
            //   break;
            case IDM_GO_PAUSE:
               {
                  HWND path_hwnd (GetDlgItem (hwnd, IDM_PATH));

                  auto path = w::get_window_text (path_hwnd);

                  if (!path.empty ())
                  {
                     w::output_debug_string (_T ("FolderSize.Win32 : New job started"));
                     s_state = state::ptr ();

                     s_state = state::ptr (new state (hwnd, path));
                     invalidate_folder_tree_area (hwnd);
                  }
               }
               break;
            case IDM_STOP:
               w::output_debug_string  (_T ("FolderSize.Win32 : Job terminated"));
               if (s_state.get ())
               {
                  s_state->traverser.stop_traversing ();
               }
               break;
            default:
               return DefWindowProc (hwnd, message, wParam, lParam);
            }
            break;
         case WM_PAINT:
            hdc = BeginPaint (hwnd, &ps);

            {
               if (s_state.get ())
               {
                  auto rect = calculate_window_coordinate (
                        hwnd
                     ,  s_folder_tree
                     );

                  s_state->painter.paint (
                        s_state->traverser.get_root ()
                     ,  hwnd
                     ,  hdc
                     ,  rect
                     ,  create_vector (0.0   , 0.0)
                     ,  create_vector (1.0   , 1.0)
                     );

               }
            }
            EndPaint (hwnd, &ps);
            break;
         case WM_DESTROY:
            PostQuitMessage (0);
            break;
         case WM_SIZE:
            update_child_window_positions (hwnd);
            invalidate_folder_tree_area (hwnd);
            break;
         case WM_GETMINMAXINFO:
            {
               MINMAXINFO * const minMaxInfo = reinterpret_cast<MINMAXINFO*> (lParam);
               if (minMaxInfo)
               {
                  minMaxInfo->ptMinTrackSize.x = 400;
                  minMaxInfo->ptMinTrackSize.y = 400;
               }
            }
            break;
         default:
            return DefWindowProc (hwnd, message, wParam, lParam);
         }
         return 0;
      }
      // ---------------------------------------------------------------------

      // ----------------------------------------------------------------------
      ATOM register_window_class (HINSTANCE const instance)
      {
         WNDCLASSEX wcex      = {0};

         wcex.cbSize          = sizeof (WNDCLASSEX);

         wcex.style           = CS_HREDRAW | CS_VREDRAW;
         wcex.lpfnWndProc     = window_process;
         wcex.cbClsExtra      = 0;
         wcex.cbWndExtra      = 0;
         wcex.hInstance       = instance;
         wcex.hIcon           = LoadIcon (instance, MAKEINTRESOURCE (IDI_FOLDERSIZEWIN32));
         wcex.hCursor         = LoadCursor (NULL, IDC_ARROW);
         wcex.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 0);
         wcex.lpszMenuName    = NULL;
         wcex.lpszClassName   = s_window_class;
         wcex.hIconSm         = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_SMALL));

         return RegisterClassEx (&wcex);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      bool init_instance (HINSTANCE const instance, int const command_show)
      {
         HWND hwnd   = {0};

         INITCOMMONCONTROLSEX InitCtrls = {0};
      	InitCtrls.dwSize = sizeof(InitCtrls);
         InitCtrls.dwICC = ICC_WIN95_CLASSES;
      	InitCommonControlsEx(&InitCtrls);

         s_instance = instance; // Store instance handle in our global variable

         hwnd = CreateWindowEx (
               WS_EX_APPWINDOW | WS_EX_COMPOSITED /*| WS_EX_LAYERED*/
            ,  s_window_class
            ,  s_title
            ,  WS_OVERLAPPEDWINDOW
            ,  CW_USEDEFAULT
            ,  0
            ,  CW_USEDEFAULT
            ,  0
            ,  NULL
            ,  NULL
            ,  instance
            ,  NULL
            );

         if (!hwnd)
         {
            return false;
         }

         //MARGINS margins = {0};
         //margins.cxLeftWidth     = -1  ;
         //margins.cxRightWidth    = -1  ;
         //margins.cyTopHeight     = -1  ;
         //margins.cyBottomHeight  = -1  ;

         //if (DwmExtendFrameIntoClientArea.is_valid ())
         //{
         //   auto extend_frame_result = DwmExtendFrameIntoClientArea.value (
         //         hwnd
         //      ,  &margins
         //      );
         //   UNUSED_VARIABLE (extend_frame_result);
         //}

         auto sz = get_client_size (hwnd);

         for_all_child_windows (
            [hwnd,instance, sz] (child_window & wc)
            {
               LPCTSTR window_class = NULL;

               switch (wc.window_type)
               {
               case window_type::button:
                  window_class = _T("BUTTON");
                  break;
               case window_type::edit:
                  window_class = _T("EDIT");
                  break;
               case window_type::static_:
                  window_class = _T("STATIC");
                  break;
               case window_type::nowindow:
               default:
                  break;
               }

               if (window_class)
               {
                  auto rect = calculate_window_coordinate (
                        sz
                     ,  wc);

                  wc.hwnd = CreateWindowEx (
                     0        | wc.extended_style
                  ,  window_class
                  ,  wc.title
                  ,  WS_CHILD | wc.style
                  ,  rect.left
                  ,  rect.top
                  ,  rect.right - rect.left
                  ,  rect.bottom - rect.top
                  ,  hwnd
                  ,  reinterpret_cast<HMENU> (wc.id)
                  ,  instance
                  ,  NULL
                  );

                  ShowWindow (wc.hwnd, SW_SHOW);
               }
            });

         ShowWindow (hwnd, command_show);

         return true;
      }
      // ----------------------------------------------------------------------

      // ---------------------------------------------------------------------
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   int application_main_loop (
         HINSTANCE const instance
      ,  HINSTANCE const previous_instance
      ,  LPTSTR const    command_line
      ,  int const       command_show)
   {
      UNREFERENCED_PARAMETER (previous_instance);
      UNREFERENCED_PARAMETER (command_line);

      MSG msg                    = { 0 };
      HACCEL accelerator_table   = { 0 };

      // Initialize global strings
      LoadString (instance, IDS_APP_TITLE, s_title, s_max_load_string);
      LoadString (instance, IDC_FOLDERSIZEWIN32, s_window_class, s_max_load_string);
      register_window_class (instance);

      // Perform application initialization:
      if (!init_instance (instance, command_show))
      {
         return FALSE;
      }

      accelerator_table = LoadAccelerators (instance, MAKEINTRESOURCE (IDR_ACCELERATOR));

      // Main message loop:
      while (GetMessage (&msg, NULL, 0, 0))
      {
         if (!TranslateAccelerator (msg.hwnd, accelerator_table, &msg))
         {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
         }
      }

      s_state = state::ptr ();

      return msg.wParam;
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
