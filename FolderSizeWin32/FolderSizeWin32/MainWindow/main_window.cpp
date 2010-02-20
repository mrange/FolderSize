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
#include <shellapi.h>
#include <shlobj.h>
// ----------------------------------------------------------------------------
#include <algorithm>
#include <functional>
#include <iterator>
// ----------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
// ----------------------------------------------------------------------------
#include "../messages.hpp"
#include "../Painter/painter.hpp"
#include "../theme.hpp"
#include "../Traverser/traverser.hpp"
#include "../utility.hpp"
#include "../view_transform.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
#undef max
#undef min
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#pragma comment(lib, "comctl32.lib")
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
namespace main_window
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace b    = boost           ;
   namespace f    = folder          ;
   namespace l    = linear          ;
   namespace p    = painter         ;
   namespace s    = std             ;
   namespace st   = s::tr1          ;
   namespace t    = traverser       ;
   namespace vt   = view_transform  ;
   namespace w    = win32           ;
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
      //typedef HRESULT (WINAPI *DwmExtendFrameIntoClientAreaPtr)(HWND, MARGINS const *);
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      namespace window_type
      {
         enum type
         {
            nowindow       ,
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

         HWND                 hwnd;
      };
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      int const            max_load_string                        = 128;
      int const            la                                     = 0x00000000;
      int const            ra                                     = 0x80000000;
      vt::vector const     start_centre                           = vt::create_vector (0.00,0.00);
      vt::vector const     start_zoom                             = vt::create_vector (0.99,0.99);
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      struct state
      {
         typedef s::auto_ptr<state> ptr               ;

         vt::vector                 centre            ;
         vt::vector                 zoom              ;

         t::traverser               traverser         ;
         p::painter                 painter           ;

         b::optional<POINT>         mouse_current_coord  ;

         state (
               HWND const           main_hwnd
            ,  w::tstring const &   path
            )
            :  centre         (start_centre      )
            ,  zoom           (start_zoom        )
            ,  traverser      (main_hwnd  , path   )
         {
         }
      };
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      HINSTANCE            s_instance                             = NULL;
      HWND                 s_main_window                          = {0};
      TCHAR                s_window_class    []                   = _T ("FOLDERSIZEWIN32");
      TCHAR                s_title           [max_load_string]  = {0};
      child_window         s_child_window    []                   =
      {
         {  IDM_GO_PAUSE   , 8      , 8         , 8 + 104   , 8 + 32 , window_type::button   , BS_DEFPUSHBUTTON   ,  0  },
         {  IDM_STOP       , 120    , 8         , 120 + 82  , 8 + 32 , window_type::button   , BS_PUSHBUTTON      ,  0  },
         {  IDM_BROWSE     , 210    , 8         , 210 + 82  , 8 + 32 , window_type::button   , BS_PUSHBUTTON      ,  0  },
         {  IDM_PATH       , 300    , 11        , ra | 8    , 8 + 29 , window_type::edit     , WS_BORDER          ,  0  },
         {  IDM_FOLDERTREE , 0      , 48        , ra | 0    , ra | 22, window_type::nowindow , 0                  ,  0  },
         {  IDM_INFO       , 8      , ra | 22   , ra | 8    , ra | 0 , window_type::static_  , SS_CENTER          ,  0  },
      };

      child_window &       s_folder_tree                          = s_child_window[4];

      state::ptr           s_state;

      //w::dll               s_dwm_dll (_T ("DwmApi"));
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      //w::function_pointer<DwmExtendFrameIntoClientAreaPtr> DwmExtendFrameIntoClientArea (
      //      s_dwm_dll.value
      //   ,  "DwmExtendFrameIntoClientArea");
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      namespace iteration_control
      {
         enum type
         {
            continue_   ,
            break_      ,
         };
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      template<typename T>
      int const size_of_array (T const & a)
      {
         return sizeof (a) / sizeof (a[0]);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      template<typename TPredicate>
      void for_all_child_windows (TPredicate const predicate)
      {
         auto size = size_of_array (s_child_window);
         for (auto iter = 0; iter < size; ++iter)
         {
            child_window & wc = s_child_window[iter];

            if (predicate (wc) == iteration_control::break_)
            {
               return;
            }
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      template<typename TPredicate>
      void circle_child_windows (
            bool const forward
         ,  int const start_index
         ,  TPredicate const predicate)
      {
         auto size = size_of_array (s_child_window);
         auto increment = forward ? 1 : -1;

         auto iter = start_index % size;

         do
         {
            child_window & wc = s_child_window[iter];

            if (predicate (wc) == iteration_control::break_)
            {
               return;
            }

            iter = (iter + increment) % size;
         }
         while (start_index != iter);

         return;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      template<typename TPredicate>
      int const find_child_window (TPredicate const predicate)
      {
         int size = size_of_array (s_child_window);
         for (int iter = 0; iter < size; ++iter)
         {
            child_window & wc = s_child_window[iter];

            if (predicate (wc))
            {
               return iter;
            }
         }
         return -1;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void build_path (
            std::vector<TCHAR> &    path
         ,  w::tstring const &      root_path
         ,  f::folder const * const folder
         )
      {
         if (folder && folder->parent)
         {
            build_path (
                  path
               ,  root_path
               ,  folder->parent
               );
            path.push_back (_T ('\\'));
            s::copy (
                  folder->name.begin ()
               ,  folder->name.end ()
               ,  s::back_inserter (path)
               );
         }
         else
         {
            auto find_last_non_slash = root_path.find_last_not_of ( _T ("\\/"));

            if (w::tstring::npos == find_last_non_slash)
            {
               s::copy (
                     root_path.begin ()
                  ,  root_path.end ()
                  ,  s::back_inserter (path)
                  );
            }
            else
            {
               s::copy (
                     root_path.begin ()
                  ,  root_path.begin () + find_last_non_slash + 1
                  ,  s::back_inserter (path)
                  );
            }
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      SIZE const calculate_size (RECT const & rect)
      {
         SIZE sz = {0};
         sz.cx = rect.right - rect.left;
         sz.cy = rect.bottom - rect.top;

         return sz;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      SIZE const get_client_size (HWND const hwnd)
      {
         RECT rect = {0};
         auto result = GetClientRect (hwnd, &rect);
         UNUSED_VARIABLE (result);

         BOOST_ASSERT (result);
         
         return calculate_size (rect);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      RECT const calculate_window_coordinate (
            SIZE const & main_window_size
         ,  child_window const & child_window)
      {
         auto calculate_coord = [] (int c, int rc) -> int
         {
            if ((c & ra) == ra)
            {
               return rc - (c & ~ra);
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
            [&sz] (child_window const & wc) -> iteration_control::type
            {
               auto child_window = wc.hwnd;
               
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

               return iteration_control::continue_;
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

         auto result = InvalidateRect (
               hwnd
            ,  &rect
            , FALSE
            );
         UNUSED_VARIABLE (result);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void gradient_fill (
            HDC const hdc
         ,  RECT const & rect
         ,  COLORREF const top_color
         ,  COLORREF const bottom_color
         )
      {
            TRIVERTEX vertex[2] = {0};
            vertex[0].x     = rect.left;
            vertex[0].y     = rect.top;
            vertex[0].Red   = GetRValue (top_color) << 8;
            vertex[0].Green = GetGValue (top_color) << 8;
            vertex[0].Blue  = GetBValue (top_color) << 8;
            vertex[0].Alpha = 0xFF00;

            vertex[1].x     = rect.right;
            vertex[1].y     = rect.bottom;
            vertex[1].Red   = GetRValue (bottom_color) << 8;
            vertex[1].Green = GetGValue (bottom_color) << 8;
            vertex[1].Blue  = GetBValue (bottom_color) << 8;
            vertex[1].Alpha = 0xFF00;

            GRADIENT_RECT gRect = {0};
            gRect.UpperLeft  = 0;
            gRect.LowerRight = 1;

            GdiGradientFill (
                  hdc
               ,  vertex
               ,  2
               ,  &gRect
               ,  1
               ,  GRADIENT_FILL_RECT_V
               );
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
         switch (message)
         {
         case WM_TIMER:
         case WM_MOUSEMOVE:
         case WM_PAINT:
            break;
         default:
            TCHAR buffer[max_load_string] = {0};
            _stprintf_s (
                  buffer
               ,  _T ("WM2: 0x%08X, 0x%04X, 0x%08X, 0x%08X")
               ,  hwnd
               ,  message
               ,  wParam
               ,  lParam
               );
            w::output_debug_string (buffer);
            break;
         }

         auto sz           = get_client_size (hwnd);
         LRESULT l_result  = 0;

         auto folder_tree_rect = calculate_window_coordinate (
               hwnd
            ,  s_folder_tree
            );

         auto do_default_proc    = false;
         auto folder_tree_size   = calculate_size (folder_tree_rect);

         switch (message)
         {
         case messages::new_view_available:
            invalidate_folder_tree_area (hwnd);
            break;
         case messages::folder_structure_changed:
            if (s_state.get ())
            {
               s_state->painter.do_request (
                     s_state->traverser.get_root ()
                  ,  hwnd
                  ,  s_state->traverser.get_processed_folder_count ()
                  ,  s_state->traverser.get_unprocessed_folder_count ()
                  ,  folder_tree_rect
                  ,  s_state->centre
                  ,  s_state->zoom
                  );
            }
            break;
         case WM_CTLCOLORSTATIC:
            {
               auto hdc = reinterpret_cast<HDC> (wParam);
               SetBkMode (hdc, TRANSPARENT);
               l_result = reinterpret_cast<LRESULT> (reinterpret_cast<HBRUSH> (GetStockObject (NULL_BRUSH)));
               break;
            }
            break;
         case WM_CTLCOLORBTN:
            {
               //auto hdc = reinterpret_cast<HDC> (wParam);
               l_result = reinterpret_cast<LRESULT> (theme::background_brush.value);
               break;
            }
            break;
         case WM_COMMAND:
            {
               auto wmId    = LOWORD (wParam);
               auto wmEvent = HIWORD (wParam);
               // Parse the menu selections:
               
               TCHAR buffer[max_load_string] = {0};
               _stprintf_s (
                     buffer
                  ,  _T ("WM_COMMAND: %d, %d")
                  ,  wmId
                  ,  wmEvent);

               w::output_debug_string (buffer);

               switch (wmId)
               {
               case IDM_GO_PAUSE:
                  {
                     auto const path_hwnd = GetDlgItem (hwnd, IDM_PATH);

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
                case IDM_BROWSE:
                  {
                     auto const path_hwnd = GetDlgItem (hwnd, IDM_PATH);

                     auto path = w::get_window_text (path_hwnd);

                     if (!path.empty ())
                     {
                        ShellExecute (
                              NULL
                           ,  _T ("explore")
                           ,  path.c_str ()
                           ,  NULL
                           ,  NULL
                           ,  SW_SHOWNORMAL);
                     }
                  }
                  break;
               case IDM_PATH:
                  break;
               case IDM_NEXT_CONTROL:
                  {
                     auto focus_hwnd = GetFocus ();
                     auto index = find_child_window (
                        [focus_hwnd] (child_window const & wc)
                        {
                           return wc.hwnd == focus_hwnd;
                        });

                     circle_child_windows (
                           true
                        ,  index + 1
                        ,  [] (child_window const & wc) -> iteration_control::type
                           {
                              switch (wc.window_type)
                              {
                              case window_type::nowindow:
                              case window_type::static_:
                                 return iteration_control::continue_;
                              default:
                                 SetFocus (wc.hwnd);
                                 return iteration_control::break_;
                              }
                           });

                     //SendMessage (
                     //      s_main_window
                     //   ,  WM_CHANGEUISTATE
                     //   ,  UIS_INITIALIZE | UISF_HIDEFOCUS << 16
                     //   ,  0
                     //   );
                  }
               default:
                  do_default_proc = true;
                  break;
               }
            }
            break;
         case WM_PAINT:
            {
               w::paint_device_context pdc (hwnd);

               if (s_state.get ())
               {
                  s_state->painter.paint (
                        s_state->traverser.get_root ()
                     ,  hwnd
                     ,  pdc.hdc
                     ,  s_state->traverser.get_processed_folder_count ()
                     ,  s_state->traverser.get_unprocessed_folder_count ()
                     ,  folder_tree_rect
                     ,  s_state->centre
                     ,  s_state->zoom
                     );

               }
               else
               {
                  FillRect (
                        pdc.hdc
                     ,  &folder_tree_rect
                     ,  theme::folder_tree::background_brush.value
                     );

                  w::select_object select_font (pdc.hdc, theme::default_big_font.value);

                  SetBkColor (
                        pdc.hdc
                     ,  theme::folder_tree::background_color
                     );

                  SetTextColor (
                        pdc.hdc
                     ,  theme::folder_tree::folder_background_color
                     );

                  DrawText (
                        pdc.hdc
                     ,  _T ("Click Go to start...")
                     ,  -1
                     ,  &folder_tree_rect
                     ,  DT_VCENTER | DT_CENTER | DT_SINGLELINE
                     );

               }

               {
                  RECT rect = {0};
                  rect.left      = 0;
                  rect.top       = 0;
                  rect.right     = sz.cx;
                  rect.bottom    = folder_tree_rect.top;

                  gradient_fill (
                        pdc.hdc
                     ,  rect
                     ,  theme::background_gradient_top_color
                     ,  theme::background_gradient_bottom_color
                     );
               }

               {
                  RECT rect = {0};
                  rect.left      = 0;
                  rect.top       = folder_tree_rect.bottom;
                  rect.right     = sz.cx;
                  rect.bottom    = sz.cy;

                  gradient_fill (
                        pdc.hdc
                     ,  rect
                     ,  theme::background_gradient_top_color
                     ,  theme::background_gradient_bottom_color
                     );
               }

            }
            break;
         case WM_MOUSEMOVE:
            {
               auto mouse_coord = w::get_mouse_coordinate (lParam);

               if (
                     s_state.get () 
                  && s_state->mouse_current_coord 
                  && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  auto mouse_current_coord = *s_state->mouse_current_coord;

                  auto reverse_transform = vt::view_to_screen (
                        vt::transform_direction::reverse
                     ,  vt::create_vector (folder_tree_size.cx, folder_tree_size.cy)
                     ,  s_state->centre
                     ,  s_state->zoom
                     );

                  auto current_cord    = l::shrink_vector (
                        reverse_transform * vt::create_extended_vector (
                           mouse_current_coord.x
                        ,  mouse_current_coord.y)
                     );

                  auto cord            = l::shrink_vector (
                        reverse_transform * vt::create_extended_vector (
                           mouse_coord.x
                        ,  mouse_coord.y)
                     );

                  auto diff = cord - current_cord;

                  s_state->centre = s_state->centre - diff;
                  s_state->mouse_current_coord = mouse_coord;

                  invalidate_folder_tree_area (hwnd);
               }
            }
            break;
         case WM_LBUTTONDOWN:
            {
               auto mouse_coord = w::get_mouse_coordinate (lParam);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  s_state->mouse_current_coord = mouse_coord;
               }
            }
            break;
         case WM_LBUTTONUP:
            {
               auto mouse_coord = w::get_mouse_coordinate (lParam);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  auto adjusted_coord  =  mouse_coord;
                  adjusted_coord.x     -= folder_tree_rect.left;
                  adjusted_coord.y     -= folder_tree_rect.top;
                  auto found_folder = s_state->painter.hit_test (adjusted_coord);

                  if (found_folder)
                  {
                     std::vector<TCHAR> path_builder;
                     path_builder.reserve (MAX_PATH);
                     build_path (
                           path_builder
                        ,  s_state->traverser.get_root_path ()
                        ,  found_folder
                        );
                     w::tstring const path (path_builder.begin (), path_builder.end ());

                     auto const path_hwnd = GetDlgItem (hwnd, IDM_PATH);

                     if (path_hwnd)
                     {
                        SetWindowText (path_hwnd, path.c_str ());
                     }
                  }
               }
            }
            // Intentional fall-through
            // break;
         case WM_KILLFOCUS:
            {
               if (s_state.get ())
               {
                  s_state->mouse_current_coord = boost::optional<POINT> ();
               }
            }
            break;
         case WM_RBUTTONUP:
            {
               auto mouse_coord = w::get_mouse_coordinate (lParam);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  s_state->centre   = start_centre  ;
                  s_state->zoom     = start_zoom    ;

                  invalidate_folder_tree_area (hwnd);
               }
            }
            break;
         case WM_MOUSEWHEEL:
            {
               auto scroll = static_cast<short> (HIWORD (wParam)) / WHEEL_DELTA;
               auto mouse_coord = w::get_client_mouse_coordinate (hwnd, lParam);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  auto scale = s::pow (1.2, scroll);

                  auto reverse_transform = vt::view_to_screen (
                        vt::transform_direction::reverse
                     ,  vt::create_vector (folder_tree_size.cx, folder_tree_size.cy)
                     ,  s_state->centre
                     ,  s_state->zoom
                     );

                  auto x = mouse_coord.x - folder_tree_rect.left;
                  auto y = mouse_coord.y - folder_tree_rect.top;

                  auto centre_of_screen_x = folder_tree_size.cx / 2;
                  auto centre_of_screen_y = folder_tree_size.cy / 2;

                  auto coord              = l::shrink_vector (
                        reverse_transform * vt::create_extended_vector (x, y)
                     );

                  auto centre_of_screen   = l::shrink_vector (
                        reverse_transform * vt::create_extended_vector (centre_of_screen_x, centre_of_screen_y)
                     );

                  auto diff                  = centre_of_screen - coord;
                  auto new_centre_of_screen  = coord + l::scale_vector (1 / scale, diff);

                  auto new_zoom     = l::scale_vector (
                        scale
                        ,  s_state->zoom
                     );

                  s_state->centre   = new_centre_of_screen  ;
                  s_state->zoom     = new_zoom              ;

                  invalidate_folder_tree_area (hwnd);
               }
            }
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
            do_default_proc = true;
            break;
         }

         if (do_default_proc)
         {
            l_result = DefWindowProc (hwnd, message, wParam, lParam);
         }

         return l_result;
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
         wcex.hIcon           = LoadIcon (instance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));
         wcex.hCursor         = LoadCursor (NULL, IDC_ARROW);
         wcex.hbrBackground   = NULL;
         wcex.lpszMenuName    = NULL;
         wcex.lpszClassName   = s_window_class;
         wcex.hIconSm         = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_SMALL));

         return RegisterClassEx (&wcex);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      bool init_instance (HINSTANCE const instance, int const command_show)
      {
         INITCOMMONCONTROLSEX InitCtrls = {0};
      	InitCtrls.dwSize = sizeof (InitCtrls);
         InitCtrls.dwICC = ICC_WIN95_CLASSES;
      	InitCommonControlsEx (&InitCtrls);

         s_instance = instance; // Store instance handle in our global variable

         s_main_window = CreateWindowEx (
               WS_EX_APPWINDOW | WS_EX_COMPOSITED /*| WS_EX_LAYERED*/
            ,  s_window_class
            ,  s_title
            ,     WS_OVERLAPPEDWINDOW
               |  WS_VISIBLE
               |  WS_TABSTOP
            ,  CW_USEDEFAULT
            ,  0
            ,  CW_USEDEFAULT
            ,  0
            ,  NULL
            ,  NULL
            ,  instance
            ,  NULL
            );

         if (!s_main_window)
         {
            return false;
         }

         SendMessage (
               s_main_window
            ,  WM_SETFONT
            ,  reinterpret_cast<WPARAM> (theme::default_font.value)
            ,  FALSE);

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

         auto sz = get_client_size (s_main_window);

         for_all_child_windows (
            [instance, sz] (child_window & wc) -> iteration_control::type
            {
               LPCTSTR window_class = NULL;

               switch (wc.window_type)
               {
               case window_type::button:
                  window_class = _T ("BUTTON");
                  break;
               case window_type::edit:
                  window_class = _T ("EDIT");
                  break;
               case window_type::static_:
                  window_class = _T ("STATIC");
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

                  auto size = calculate_size (rect);

                  TCHAR buffer[max_load_string]  = {0};

                  LoadString (
                        instance
                     ,  wc.id
                     ,  buffer
                     ,  max_load_string
                     );

                  wc.hwnd = CreateWindowEx (
                     wc.extended_style
                  ,  window_class
                  ,  buffer
                  ,     wc.style 
                     |  WS_CHILD 
                     |  WS_VISIBLE
                     |  WS_TABSTOP
                  ,  rect.left
                  ,  rect.top
                  ,  size.cx
                  ,  size.cy
                  ,  s_main_window
                  ,  reinterpret_cast<HMENU> (wc.id)
                  ,  instance
                  ,  NULL
                  );

               SendMessage (
                     wc.hwnd
                  ,  WM_SETFONT
                  ,  reinterpret_cast<WPARAM> (theme::default_font.value)
                  ,  FALSE);
               }

               return iteration_control::continue_;
            });

         TCHAR known_folder_path[MAX_PATH] = {0};

         auto get_folder_hr = SHGetFolderPath (
               s_main_window
            ,  CSIDL_PERSONAL
            ,  NULL
            ,  SHGFP_TYPE_CURRENT 
            ,  known_folder_path
            );

         auto path_hwnd = GetDlgItem (s_main_window, IDM_PATH);

         if (SUCCEEDED (get_folder_hr) && path_hwnd)
         {
            SetWindowText (path_hwnd, known_folder_path);
         }

         SendMessage (
               s_main_window
            ,  WM_CHANGEUISTATE
            ,  UIS_SET | UISF_HIDEACCEL << 16
            ,  0
            );

         ShowWindow (s_main_window, command_show);

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
      auto set_priority_class_result = SetPriorityClass (GetCurrentProcess (), IDLE_PRIORITY_CLASS);
      UNUSED_VARIABLE (set_priority_class_result);

      MSG msg                    = { 0 };
      HACCEL accelerator_table   = { 0 };

      // Initialize global strings
      LoadString (instance, IDC_FOLDERSIZEWIN32, s_title, max_load_string);
      register_window_class (instance);

      // Perform application initialization:
      if (!init_instance (instance, command_show))
      {
         return FALSE;
      }

      accelerator_table = LoadAccelerators (instance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));

      // Main message loop:
      while (GetMessage (&msg, NULL, 0, 0))
      {
         switch (msg.message)
         {
         case WM_TIMER:
         case WM_MOUSEMOVE:
            break;
         default:
            TCHAR buffer[max_load_string] = {0};
            _stprintf_s (
                  buffer
               ,  _T ("WM: 0x%08X, 0x%04X, 0x%08X, 0x%08X")
               ,  msg.hwnd
               ,  msg.message
               ,  msg.wParam
               ,  msg.lParam
               );
            w::output_debug_string (buffer);
            break;
         }

         auto process_message = true;

         switch (msg.message)
         {
         case WM_SYSKEYDOWN:
            switch (msg.wParam)
            {
            case VK_MENU:
               {
                  SendMessage (
                        s_main_window
                     ,  WM_CHANGEUISTATE
                     ,  UIS_INITIALIZE | UISF_HIDEACCEL << 16
                     ,  0
                     );
                  break;
               }
            }
            break;
         case WM_SYSKEYUP:
            switch (msg.wParam)
            {
            case VK_MENU:
               {
                  SendMessage (
                        s_main_window
                     ,  WM_CHANGEUISTATE
                     ,  UIS_SET | UISF_HIDEACCEL << 16
                     ,  0
                     );
                  break;
               }
            }
            break;
         }

         if (process_message && !TranslateAccelerator (msg.hwnd, accelerator_table, &msg))
         {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
         }
      }

      s_state = state::ptr ();

      return msg.wParam;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
