// ----------------------------------------------------------------------------
#include "stdafx.h"
#include "../resource.h"
// ----------------------------------------------------------------------------
#include <atlctrls.h>
// ----------------------------------------------------------------------------
#include <functional>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../Painter/Painter.hpp"
#include "../Traverser/Traverser.hpp"
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
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace window_style
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
      window_style::type   style;
      LPCTSTR              title;

      HWND                 hwnd;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct state
   {
      typedef s::auto_ptr<state> ptr      ;
      t::traverser               traverser;
      p::painter                 painter  ;

      static p::painter::folder_getter  get_folder_getter (
         t::traverser const & t)
      {
         return [&t] () { return t.get_root (); };
      }

      state (LPCTSTR const path)
         :  traverser   (path)
         ,  painter     (get_folder_getter (traverser))
      {
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   const int            s_max_load_string                      = 100;
   HINSTANCE            s_instance                             = NULL;
   TCHAR                s_title           [s_max_load_string]  = {0};
   TCHAR                s_window_class    [s_max_load_string]  = {0};
   child_window         s_child_window    []                   =
   {
      {  IDM_GO_PAUSE   , 8      , 8   , 8 + 80    , 8 + 32 , window_style::button  , _T("&Go")    },
      {  IDM_STOP       , 100    , 8   , 100 + 80  , 8 + 32 , window_style::button  , _T("&Stop")  },
      {  IDM_PATH       , 200    , 8   , -8        , 8 + 32 , window_style::edit    , _T(".")      },
      {  IDM_FOLDERTREE , 8      , 48  , -8        , -8     , window_style::static_ , _T("")       },
      {0},
   };
   state::ptr           s_state;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   typedef st::function<void (child_window&)>  child_window_predicate;
   void for_all_child_windows (child_window_predicate predicate)
   {
      for (int iter = 0; true; ++iter)
      {
         child_window & wc = s_child_window[iter];

         if (wc.style == window_style::nowindow)
         {
            return;
         }

         predicate (wc);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   int                  application_main_loop (
                           HINSTANCE instance,
                           HINSTANCE previous_instance,
                           LPTSTR    command_line,
                           int       command_show);
   ATOM                 register_window_class (HINSTANCE instance);
   bool                 init_instance (HINSTANCE, int);
   void                 update_child_window_positions (HWND);
   LRESULT CALLBACK     window_process (HWND, UINT, WPARAM, LPARAM);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   int                  application_main_loop (
                           HINSTANCE instance,
                           HINSTANCE previous_instance,
                           LPTSTR    command_line,
                           int       command_show)
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

      accelerator_table = LoadAccelerators (instance, MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32));

      // Main message loop:
      while (GetMessage (&msg, NULL, 0, 0))
      {
         if (!TranslateAccelerator (msg.hwnd, accelerator_table, &msg))
         {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
         }
      }

      return msg.wParam;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   ATOM                 register_window_class (HINSTANCE instance)
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
      wcex.lpszMenuName    = MAKEINTRESOURCE (IDC_FOLDERSIZEWIN32);
      wcex.lpszClassName   = s_window_class;
      wcex.hIconSm         = LoadIcon (wcex.hInstance, MAKEINTRESOURCE (IDI_SMALL));

      return RegisterClassEx (&wcex);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   bool                 init_instance (HINSTANCE instance, int command_show)
   {
      HWND hwnd   = {0};

      s_instance = instance; // Store instance handle in our global variable

      hwnd = CreateWindow (
         s_window_class, 
         s_title, 
         WS_OVERLAPPEDWINDOW,
         CW_USEDEFAULT, 
         0, 
         CW_USEDEFAULT, 
         0, 
         NULL, 
         NULL, 
         instance, 
         NULL);

      if (!hwnd)
      {
         return false;
      }

      for_all_child_windows (
         [hwnd] (child_window & wc)
         {
            LPCTSTR window_class = NULL;

            switch (wc.style)
            {
            case window_style::button:
               window_class = _T("BUTTON");
               break;
            case window_style::edit:
               window_class = _T("EDIT");
               break;
            case window_style::static_:
               window_class = _T("STATIC");
               break;
            case window_style::nowindow:
            default:
               break;
            }

            CWindow window;
            wc.hwnd = window.Create (
               window_class,
               hwnd,
               NULL,
               NULL,
               WS_CHILD,
               0,
               wc.id);
            //window.MoveWindow (wc.left, wc.top, wc.right - wc.left, wc.bottom - wc.top);
            window.SetWindowText (wc.title);
            window.ShowWindow (SW_SHOW);
         });

      update_child_window_positions (hwnd);

      ShowWindow (hwnd, command_show);
      UpdateWindow (hwnd);

      return true;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void                 update_child_window_positions (HWND hwnd)
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

      RECT rect = {0};
      if (GetClientRect (hwnd, &rect))
      {
         for_all_child_windows (
            [&rect,calculate_coord] (child_window & wc)
            {
               HWND child_window = wc.hwnd;
               
               int width         = rect.right                  - rect.left;
               int height        = rect.bottom                 - rect.top;

               int real_left     = calculate_coord (wc.left    , width);
               int real_top      = calculate_coord (wc.top     , height);
               int real_right    = calculate_coord (wc.right   , width);
               int real_bottom   = calculate_coord (wc.bottom  , height);

               CWindow window(child_window);
               window.MoveWindow (
                  real_left,
                  real_top,
                  real_right     - real_left,
                  real_bottom    - real_top);
            });
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   LRESULT CALLBACK     window_process (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
   {
      int wmId       = {0};
      int wmEvent    = {0};
      PAINTSTRUCT ps = {0};
      HDC hdc        = {0};

      switch (message)
      {
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
               CWindow window (GetDlgItem (hwnd, IDM_PATH));

               if (window)
               {
                  CString str;
                  window.GetWindowText (str);

                  if (str.GetLength () > 0)
                  {
                     s_state = state::ptr (new state (str));
                  }
               }
            }
            break;
         case IDM_STOP:
            s_state = state::ptr ();

            break;
         default:
            return DefWindowProc (hwnd, message, wParam, lParam);
         }
         break;
      case WM_PAINT:
         hdc = BeginPaint (hwnd, &ps);
         // TODO: Add any drawing code here...
         EndPaint (hwnd, &ps);
         break;
      case WM_DESTROY:
         PostQuitMessage (0);
         break;
      case WM_SIZE:
         update_child_window_positions (hwnd);
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
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
