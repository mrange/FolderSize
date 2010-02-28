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
   namespace u    = utility         ;
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
            combo          ,
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
      vt::vector const           start_centre                     = vt::create_vector (0.00,0.00);
      vt::vector const           start_zoom                       = vt::create_vector (0.99,0.99);
      int const                  string_buffer_size               = 128;
      TCHAR const                window_class    []             = _T ("FOLDERSIZEWIN32");
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
      HINSTANCE                  s_instance                       = NULL;
      HWND                       s_main_window                    = NULL;
      p::select_property::type   s_select_property                = p::select_property::physical_size;

      // la = left aligned
      int const                  la                               = 0x00000000;
      // ra = left aligned
      int const                  ra                               = 0x80000000;

      child_window               s_child_window    []             =
      {
         {  IDM_GO_PAUSE   , la | 8    , la | 8    , la | 8 + 104    , la | 8 + 32  , window_type::button   , BS_DEFPUSHBUTTON                  ,  0  },
         {  IDM_STOP       , la | 120  , la | 8    , la | 120 + 82   , la | 8 + 32  , window_type::button   , BS_PUSHBUTTON                     ,  0  },
         {  IDM_BROWSE     , la | 210  , la | 8    , la | 210 + 82   , la | 8 + 32  , window_type::button   , BS_PUSHBUTTON                     ,  0  },
         {  IDM_PATH       , la | 300  , la | 10   , ra | 108        , la | 10 + 28 , window_type::edit     , ES_AUTOHSCROLL                    ,  WS_EX_CLIENTEDGE  },
         {  IDM_SELECTOR   , ra | 100  , la | 10   , ra | 8          , la | 10 + 29 , window_type::combo    , CBS_DROPDOWNLIST | CBS_HASSTRINGS ,  0  },
         {  IDM_FOLDERTREE , la | 0    , la | 48   , ra | 0          , ra | 22      , window_type::nowindow , 0                                 ,  0  },
         {  IDM_INFO       , la | 8    , ra | 22   , ra | 8          , ra | 0       , window_type::static_  , SS_CENTER                         ,  0  },
      };

      child_window &             s_path                           = s_child_window[3];
      child_window &             s_selector                       = s_child_window[4];
      child_window &             s_folder_tree                    = s_child_window[5];

      state::ptr                 s_state;

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
      template<typename TPredicate>
      void for_all_child_windows (TPredicate const predicate)
      {
         auto size = u::size_of_array (s_child_window);
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
         auto size = u::size_of_array (s_child_window);
         auto increment = forward ? 1 : -1;

         auto iter = (start_index + increment + size) % size;

         if (iter < 0)
         {
            return;
         }

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
         auto size = u::size_of_array (s_child_window);
         for (auto iter = 0; iter < size; ++iter)
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
      w::tstring const adjust_path (w::tstring const & path)
      {
         auto find_last_non_ws = path.find_last_not_of (_T (" "));

         if (find_last_non_ws != w::tstring::npos)
         {
            auto path_copy = path;

            path_copy.erase (find_last_non_ws + 1);

            if (path_copy[find_last_non_ws] != '\\')
            {
               path_copy += '\\';
            }

            return path_copy;
         }
         else
         {
            return w::tstring ();
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      w::tstring const get_user_home ()
      {
         TCHAR known_folder_path[MAX_PATH] = {0};

         auto get_folder_hr = SHGetFolderPath (
               s_main_window
            ,  CSIDL_PERSONAL
            ,  NULL
            ,  SHGFP_TYPE_CURRENT 
            ,  known_folder_path
            );

         if (SUCCEEDED (get_folder_hr))
         {
            return adjust_path (known_folder_path);
         }
         else
         {
            return w::load_string_resource (IDM_PATH, _T ("C:\\Windows\\"));
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

         FS_ASSERT (result);
         
         return calculate_size (rect);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      RECT const calculate_window_coordinate (
            SIZE const & main_window_size
         ,  child_window const & child_window)
      {
         auto calculate_coord = [] (int c, int rc)
         {
            return (c & ra) == ra
               ?  rc - (c & ~ra)
               :  c;
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
            [sz] (child_window const & wc) -> iteration_control::type
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
            ,  FALSE
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
      void set_ui_state (
            int property
         )
      {
         if (s_main_window)
         {
            SendMessage (
                  s_main_window
               ,  WM_CHANGEUISTATE
               ,  UIS_SET | property << 16
               ,  0
               );
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void clear_ui_state (
            int property
         )
      {
         if (s_main_window)
         {
            SendMessage (
                  s_main_window
               ,  WM_CHANGEUISTATE
               ,  UIS_CLEAR | property << 16
               ,  0
               );
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void log_windows_message (
            LPCTSTR const  prelude
         ,  HWND const     hwnd
         ,  UINT const     message
         ,  WPARAM const   w_param
         ,  LPARAM         l_param)
      {
#ifdef _DEBUG
            TCHAR buffer[string_buffer_size] = {0};
            _stprintf_s (
                  buffer
               ,  _T ("%16s : 0x%08X, 0x%04X, 0x%08X, 0x%08X")
               ,  prelude ? prelude : _T ("WM")
               ,  hwnd
               ,  message
               ,  w_param
               ,  l_param
               );
            w::debug_string (buffer);
#endif
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      POINT const adjust_coord (
         RECT const & rect,
         POINT const & coord
         )
      {
         POINT p = {0};
         p.x = coord.x - rect.left;
         p.y = coord.y - rect.top;
         return p;
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      p::select_property::type const calculate_select_property_from_index (int index)
      {
         switch (index)
         {
         case 0:
            return p::select_property::size;
         case 1:
            return p::select_property::physical_size;
         case 2:
            return p::select_property::count;
         default:
            FS_ASSERT (false);
            return p::select_property::size;
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      void select_and_focus (HWND const hwnd)
      {
         if (hwnd)
         {
            auto text_length  = GetWindowTextLength (hwnd);
            SendMessage (hwnd, EM_SETSEL, 0, text_length);
            SetFocus (hwnd);
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      static void change_select_property (int const increment)
      {
         auto index = SendMessage(s_selector.hwnd, CB_GETCURSEL, 0, 0L);
         auto new_index = (index + increment + 3) % 3;
         SendMessage(s_selector.hwnd, CB_SETCURSEL, new_index, 0L);
         s_select_property = calculate_select_property_from_index (new_index);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      static bool is_message_interesting (UINT const message)
      {
         switch (message)
         {
         case WM_NOTIFY:
         case WM_SETCURSOR:
         case WM_TIMER:
         case 0x0118:   // WM_SYSTIMER
         case WM_MOUSEMOVE:
         case WM_MOUSELEAVE:
         case WM_NCMOUSELEAVE:
         case WM_ERASEBKGND:
         case WM_PAINT:
         case WM_PRINTCLIENT:
         case WM_NCHITTEST:
         case WM_NCPAINT:
         case WM_CTLCOLORMSGBOX:
         case WM_CTLCOLOREDIT:
         case WM_CTLCOLORLISTBOX:
         case WM_CTLCOLORBTN:
         case WM_CTLCOLORDLG:
         case WM_CTLCOLORSCROLLBAR:
         case WM_CTLCOLORSTATIC:
            return false;
         default:
            return true;
         }
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      LRESULT CALLBACK window_process (
            HWND const     hwnd
         ,  UINT const     message
         ,  WPARAM const   w_param
         ,  LPARAM const   l_param
         )
      {
         if (is_message_interesting (message))
         {
            log_windows_message (
                  _T ("MainWindow")
               ,  hwnd
               ,  message
               ,  w_param
               ,  l_param
               );
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
            WIN32_DEBUG_STRING (WIN32_PRELUDE _T (" : Received : messages::new_view_available"));
            invalidate_folder_tree_area (hwnd);
            break;
         case messages::folder_structure_changed:
            WIN32_DEBUG_STRING (WIN32_PRELUDE _T (" : Received : messages::folder_structure_changed"));
            if (s_state.get ())
            {
               s_state->painter.do_request (
                     s_state->traverser.get_root ()
                  ,  hwnd
                  ,  s_state->traverser.get_processed_folder_count ()
                  ,  s_state->traverser.get_unprocessed_folder_count ()
                  ,  s_select_property
                  ,  folder_tree_rect
                  ,  s_state->centre
                  ,  s_state->zoom
                  );
            }
            break;
         case WM_CTLCOLORSTATIC:
            {
               auto hdc = reinterpret_cast<HDC> (w_param);
               SetBkMode (hdc, TRANSPARENT);
               l_result = reinterpret_cast<LRESULT> (reinterpret_cast<HBRUSH> (GetStockObject (NULL_BRUSH)));
            }
            break;
         //case WM_CTLCOLOREDIT:
         case WM_CTLCOLORBTN:
            {
               //auto hdc = reinterpret_cast<HDC> (w_param);
               l_result = reinterpret_cast<LRESULT> (theme::background_brush.value);
            }
            break;
         case WM_COMMAND:
            {
               auto wm_id     = LOWORD (w_param);
               auto wm_event  = HIWORD (w_param);
               // Parse the menu selections:
               
               TCHAR buffer[string_buffer_size] = {0};
               _stprintf_s (
                     buffer
                  ,  WIN32_PRELUDE _T (" : WM_COMMAND: %d, %d")
                  ,  wm_id
                  ,  wm_event);

               WIN32_DEBUG_STRING (buffer);

               switch (wm_id)
               {
               case IDM_SELECT_AND_FOCUS_PATH:
                  {
                     select_and_focus (s_path.hwnd);
                  }
                  break;
               case IDM_NEXT_SELECTOR:
                  {
                     change_select_property  (1);
                     invalidate_folder_tree_area (hwnd);
                  }
                  break;
               case IDM_PREVIOUS_SELECTOR:
                  {
                     change_select_property  (-1);
                     invalidate_folder_tree_area (hwnd);
                  }
                  break;
               case IDM_GO_PAUSE:
                  {
                     auto const path_hwnd = GetDlgItem (hwnd, IDM_PATH);

                     auto path = w::get_window_text (path_hwnd);

                     path = adjust_path (path);

                     if (path.empty ())
                     {
                        path = get_user_home ();
                     }

                     SetWindowText (path_hwnd, path.c_str ());

                     w::trace_string (_T ("New job started"));
                     s_state = state::ptr ();

                     s_state = state::ptr (new state (hwnd, path));
                     invalidate_folder_tree_area (hwnd);
                  }
                  break;
               case IDM_STOP:
                  w::trace_string  (_T ("Job terminated"));
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
                     switch (wm_event)
                     {
                     case EN_SETFOCUS:
                        {
                           auto hwnd         = reinterpret_cast<HWND> (l_param);
                           select_and_focus (hwnd);
                        }
                        break;
                     }
                  break;
               case IDM_SELECTOR:
                  {
                     switch (wm_event)
                     {
                     case CBN_SELENDOK:
                     //case CBN_SELCHANGE:
                     //case CBN_SELENDCANCEL:
                        {
                           auto selection = calculate_select_property_from_index (SendMessage (s_selector.hwnd, CB_GETCURSEL, 0, 0));

                           if (selection != s_select_property)
                           {
                              s_select_property = selection;
                              invalidate_folder_tree_area (hwnd);
                           }
                        }
                        break;
                     }
                  }
                  break;
               default:
                  do_default_proc = true;
                  break;
               }
            }
            break;
         case WM_PAINT:
            {
               w::paint_device_context pdc (hwnd);

               RECT clip_box = {0};

               auto get_clipbox_result = GetClipBox (
                     pdc.hdc
                  ,  &clip_box
                  );

               if (
                     get_clipbox_result == SIMPLEREGION 
                  || get_clipbox_result == COMPLEXREGION
                  )
               {
                  if (s_state.get () && w::intersect (clip_box, folder_tree_rect))
                  {
                     WIN32_DEBUG_STRING (WIN32_PRELUDE _T (" : WM_PAINT : FolderTree"));
                     s_state->painter.paint (
                           s_state->traverser.get_root ()
                        ,  hwnd
                        ,  pdc.hdc
                        ,  s_state->traverser.get_processed_folder_count ()
                        ,  s_state->traverser.get_unprocessed_folder_count ()
                        ,  s_select_property
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
                        ,  theme::folder_tree::foreground_color
                        );

                     DrawText (
                           pdc.hdc
                        ,  theme::welcome_string.c_str ()
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

                     if (w::intersect (clip_box, rect))
                     {
                        WIN32_DEBUG_STRING (WIN32_PRELUDE _T (" : WM_PAINT : Top Gradient"));
                        gradient_fill (
                              pdc.hdc
                           ,  rect
                           ,  theme::background_gradient_top_color
                           ,  theme::background_gradient_bottom_color
                           );
                     }
                  }

                  {
                     RECT rect = {0};
                     rect.left      = 0;
                     rect.top       = folder_tree_rect.bottom;
                     rect.right     = sz.cx;
                     rect.bottom    = sz.cy;

                     if (w::intersect (clip_box, rect))
                     {
                        WIN32_DEBUG_STRING (WIN32_PRELUDE _T (" : WM_PAINT : Top Gradient"));
                        gradient_fill (
                              pdc.hdc
                           ,  rect
                           ,  theme::background_gradient_top_color
                           ,  theme::background_gradient_bottom_color
                           );
                     }
                  }
               }
            }
            break;
         case WM_MOUSEMOVE:
            {
               auto mouse_coord = w::get_mouse_coordinate (l_param);

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
         case WM_LBUTTONDBLCLK:
            {
               auto mouse_coord = w::get_mouse_coordinate (l_param);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  auto adjusted_coord  =  adjust_coord (folder_tree_rect, mouse_coord);
                  auto found_folder    = s_state->painter.hit_test (adjusted_coord);

                  if (found_folder.folder)
                  {
                     auto folder_height = found_folder.view_rect.values[3] - found_folder.view_rect.values[1];
                     if (folder_height > 0)
                     {
                        auto new_zoom = vt::create_vector (
                              1 / folder_height
                           ,  1 / folder_height
                           );

                        auto offset = l::scale_vector (0.5, l::invert_vector (new_zoom));

                        s_state->centre   = vt::create_vector (
                              found_folder.view_rect.values [0] + offset.x ()
                           ,  found_folder.view_rect.values [1] + offset.y ()
                           );
                        s_state->zoom     = l::scale_vector (start_zoom, new_zoom);
                     }
                  }
                  invalidate_folder_tree_area (hwnd);
               }
            }
            break;
         case WM_LBUTTONDOWN:
            {
               auto mouse_coord = w::get_mouse_coordinate (l_param);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  s_state->mouse_current_coord = mouse_coord;
               }
            }
            break;
         case WM_LBUTTONUP:
            {
               auto mouse_coord = w::get_mouse_coordinate (l_param);

               if (s_state.get () && w::is_inside (folder_tree_rect, mouse_coord))
               {
                  auto adjusted_coord  =  adjust_coord (folder_tree_rect, mouse_coord);
                  auto found_folder    = s_state->painter.hit_test (adjusted_coord);

                  if (found_folder.folder)
                  {
                     std::vector<TCHAR> path_builder;
                     path_builder.reserve (MAX_PATH);
                     build_path (
                           path_builder
                        ,  s_state->traverser.get_root_path ()
                        ,  found_folder.folder
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
               auto mouse_coord = w::get_mouse_coordinate (l_param);

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
               auto scroll = static_cast<short> (HIWORD (w_param)) / WHEEL_DELTA;
               auto mouse_coord = w::get_client_mouse_coordinate (hwnd, l_param);

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
               MINMAXINFO * const minMaxInfo = reinterpret_cast<MINMAXINFO*> (l_param);
               if (minMaxInfo)
               {
                  minMaxInfo->ptMinTrackSize.x = 600;
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
            l_result = DefWindowProc (hwnd, message, w_param, l_param);
         }

         return l_result;
      }
      // ---------------------------------------------------------------------

      // ----------------------------------------------------------------------
      ATOM register_window_class (HINSTANCE const instance)
      {
         WNDCLASSEX wcex      = {0};

         wcex.cbSize          = sizeof (WNDCLASSEX);

         wcex.style           = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
         wcex.lpfnWndProc     = window_process;
         wcex.cbClsExtra      = 0;
         wcex.cbWndExtra      = 0;
         wcex.hInstance       = instance;
         wcex.hIcon           = LoadIcon (instance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));
         wcex.hCursor         = LoadCursor (NULL, IDC_ARROW);
         wcex.hbrBackground   = NULL;
         wcex.lpszMenuName    = NULL;
         wcex.lpszClassName   = window_class;
         wcex.hIconSm         = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));

         return RegisterClassEx (&wcex);
      }
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      bool init_instance (
            HINSTANCE const   instance
         ,  LPCTSTR const     command_line
         ,  int const         command_show
         )
      {
         INITCOMMONCONTROLSEX InitCtrls = {0};
      	InitCtrls.dwSize = sizeof (InitCtrls);
         InitCtrls.dwICC = ICC_WIN95_CLASSES;
      	InitCommonControlsEx (&InitCtrls);

         s_instance = instance; // Store instance handle in our global variable

         s_main_window = CreateWindowEx (
               WS_EX_APPWINDOW | WS_EX_COMPOSITED /*| WS_EX_LAYERED*/
            ,  window_class
            ,  w::load_string_resource (IDC_FOLDERSIZEWIN32, window_class).c_str()
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
               case window_type::combo:
                  window_class = _T ("COMBOBOX");
                  break;
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

                  wc.hwnd = CreateWindowEx (
                     wc.extended_style
                  ,  window_class
                  ,  w::load_string_resource (wc.id).c_str()
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

         auto user_home_path = get_user_home ();

         auto path_hwnd = GetDlgItem (s_main_window, IDM_PATH);

         if (!user_home_path.empty () && path_hwnd)
         {
            SetWindowText (path_hwnd, user_home_path.c_str ());
         }

         set_ui_state   (UISF_HIDEACCEL);
         clear_ui_state (UISF_HIDEFOCUS);

         SendMessage(s_selector.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ( theme::size_string.c_str ()));
         SendMessage(s_selector.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ( theme::physical_size_string.c_str ()));
         SendMessage(s_selector.hwnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM> ( theme::count_string.c_str ()));
         SendMessage(s_selector.hwnd, CB_SETCURSEL, 1, 0L);

         SetFocus (s_path.hwnd);

         w::tstring cmd_line (command_line);

         if (!cmd_line.empty ())
         {
            SetWindowText (s_path.hwnd, cmd_line.c_str ());
            PostMessage (s_main_window, WM_COMMAND, IDM_GO_PAUSE, 0);
         }

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
      ,  LPCTSTR const   command_line
      ,  int const       command_show)
   {
      auto set_priority_class_result = SetPriorityClass (GetCurrentProcess (), IDLE_PRIORITY_CLASS);
      UNUSED_VARIABLE (set_priority_class_result);

      MSG msg                    = {0};
      HACCEL accelerator_table   = {0};

      register_window_class (instance);

      accelerator_table = LoadAccelerators (instance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));

      // Perform application initialization:
      if (!init_instance (instance, command_line, command_show))
      {
         return FALSE;
      }

      // Main message loop:
      while (GetMessage (&msg, NULL, 0, 0))
      {
         if (is_message_interesting (msg.message))
         {
            log_windows_message (
                  _T ("main_loop")
               ,  msg.hwnd
               ,  msg.message
               ,  msg.wParam
               ,  msg.lParam
               );
         }

         auto process_message = true;

         switch (msg.message)
         {
         case WM_KEYDOWN:
            switch (msg.wParam)
            {
            case VK_TAB:
               {
                  auto forward = 
                        IS_OFF (GetKeyState (VK_SHIFT), 0x8000)
                     && IS_OFF (GetKeyState (VK_RSHIFT), 0x8000);

                  auto focus_hwnd = msg.hwnd;
                  auto index = find_child_window (
                     [focus_hwnd] (child_window const & wc)
                     {
                        return wc.hwnd == focus_hwnd;
                     });

                  circle_child_windows (
                        forward
                     ,  index
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

                  set_ui_state   (UISF_HIDEACCEL);
                  process_message = false;
               }
            }
         case WM_KEYUP:
            switch (msg.wParam)
            {
            case VK_TAB:
               set_ui_state   (UISF_HIDEACCEL);
               process_message = false;
               break;
            case VK_ESCAPE:
               PostQuitMessage (0);
               process_message = false;
               break;
            }
            break;
         case WM_SYSKEYDOWN:
            // WM_SYSKEY* redirect to main window in order to get accelerators to work
            if (IS_ON (msg.lParam, 0x20000000))
            {
               msg.hwnd = s_main_window;
            }
            switch (msg.wParam)
            {
            case VK_MENU:
               {
                  clear_ui_state (UISF_HIDEACCEL);
                  break;
               }
            }
            break;
         case WM_SYSKEYUP:
            // WM_SYSKEY* redirect to main window in order to get accelerators to work
            if (IS_ON (msg.lParam, 0x20000000))
            {
               msg.hwnd = s_main_window;
            }
            switch (msg.wParam)
            {
            case VK_MENU:
               {
                  set_ui_state   (UISF_HIDEACCEL);
                  break;
               }
            }
            break;
         }

         if (process_message)
         {
            if (TranslateAccelerator (msg.hwnd, accelerator_table, &msg))
            {
               log_windows_message (
                     _T ("accelerator")
                  ,  msg.hwnd
                  ,  msg.message
                  ,  msg.wParam
                  ,  msg.lParam
                  );
            }
            else
            {
               TranslateMessage (&msg);
               DispatchMessage (&msg);
            }
         }

      }

      s_state = state::ptr ();

      return msg.wParam;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
