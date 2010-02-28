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
#include <process.h>
// ----------------------------------------------------------------------------
#include <tuple>
#include <vector>
// ----------------------------------------------------------------------------
#include "win32.hpp"
// ----------------------------------------------------------------------------
namespace win32
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace s    = std          ;
   namespace st   = s::tr1       ;
   namespace b    = boost        ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      bool const is_windows7_or_later ()
      {
         OSVERSIONINFO os_version_info = {0};
         os_version_info.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
         auto get_version_ex_result = GetVersionEx (
               &os_version_info
            );
         if (get_version_ex_result)
         {
            return 
               st::make_tuple (6u, 1u) <= st::make_tuple (
                     os_version_info.dwMajorVersion
                  ,  os_version_info.dwMinorVersion
                  );
         }
         else
         {
            return false;
         }
      }
   }
   // -------------------------------------------------------------------------
   bool const windows7_or_later                    = is_windows7_or_later ();              
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void trace_string (tstring const & value)
   {
      trace_string (value.c_str ());
   }

   void trace_string (LPCTSTR const value)
   {
      if (value)
      {
         // FYI: Thanks to C++0x of r-value ref this only creates one copy of 
         // tstring and appends to it. In C++98 the code below would've created
         // 2 additional tstrings
         auto output = tstring (_T ("FolderSize.Win32 : ")) + value + _T ("\r\n");
         OutputDebugString (output.c_str ());
      }
      else
      {
         OutputDebugString ( _T ("FolderSize.Win32 : NO MESSAGE\r\n"));
      }
   }

   void debug_string (tstring const & value)
   {
      debug_string (value.c_str ());
   }

   void debug_string (LPCTSTR const value)
   {
#ifdef _DEBUG
      trace_string (value);
#endif
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   file_time const to_file_time (FILETIME const ft) throw ()
   {
      return (static_cast<file_time> (ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
   }

   FILETIME const get_current_time () throw ()
   {
      SYSTEMTIME st  = {0};
      FILETIME ft    = {0};
      GetSystemTime(&st);            
      SystemTimeToFileTime(&st, &ft);
      return ft;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   tstring const get_window_text (HWND const hwnd)
   {
      if (!hwnd)
      {
         return tstring ();
      }

      auto length = GetWindowTextLength (hwnd);

      if (length > 0)
      {
         std::vector<TCHAR> buffer;
         buffer.resize (length + 1);

         auto real_length = GetWindowText (hwnd, &buffer.front (), length + 1);

         return tstring (&buffer.front (), real_length);
      }
      else
      {
         return tstring ();
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   handle::handle (HANDLE const hnd) throw ()
      :  value (hnd)
   {
   }

   handle::~handle () throw ()
   {
      if (is_valid ())
      {
         BOOL const close_result = CloseHandle (value);
         UNUSED_VARIABLE (close_result);
      }
   }

   bool const handle::is_valid () const throw ()
   {
      return value != INVALID_HANDLE_VALUE;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   dll::dll (LPCTSTR const dll_name) throw ()
      :  value (LoadLibrary (dll_name))
   {
   }

   dll::~dll () throw ()
   {
      if (is_valid ())
      {
         BOOL const free_result = FreeLibrary (value);
         UNUSED_VARIABLE (free_result);
      }
   }

   bool const dll::is_valid () const throw ()
   {
      return value != NULL;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   thread::thread (
         tstring const & thread_name_
      ,  proc const del)
      :  thread_name (thread_name_)
      ,  terminated (false)
      ,  procedure (del)
      ,  value (reinterpret_cast<HANDLE> (_beginthread (raw_proc, 0, this)))
   {
   }

   bool const thread::join (unsigned int const ms) const throw ()
   {
      if (value.is_valid ())
      {
         trace_string (_T ("Joining thread: ") + thread_name);
         auto res = WaitForSingleObject (value.value, ms);

         if (res == WAIT_OBJECT_0)
         {
            trace_string (_T ("Thread join successful: ") + thread_name);
         }
         else
         {
            trace_string (_T ("Thread join failed: ") + thread_name);
         }

         return res == WAIT_OBJECT_0;
      }
      else
      {
         return false;
      }
   }

   bool const thread::is_terminated () const throw ()
   {
      return terminated;
   }

   void thread::raw_proc (void * const ptr) throw ()
   {
      auto state = static_cast<thread *> (ptr);

      if (state)
      {
         try
         {
            trace_string (_T ("Starting thread: ") + state->thread_name);

            auto result = state->procedure ();


            if (result == EXIT_SUCCESS)
            {
               trace_string (_T ("Thread exited successfully: ") + state->thread_name);
            }
            else
            {
               trace_string (_T ("Thread exited with failure code: ") + state->thread_name);
            }

            state->terminated = true;
            _endthreadex (result);
         }
         catch (...)
         {
            trace_string (_T ("Thread threw exception: ") + state->thread_name);
            state->terminated = true;
            _endthreadex (EXIT_FAILURE);
         }
      }
      else
      {
         _endthreadex (EXIT_FAILURE);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   find_file::find_file (
      tstring const & path
      )
      :  find_file_handle (FindFirstFileEx (
               path.c_str ()
            ,  get_windows7_dependent_value (FindExInfoBasic, FindExInfoStandard)
            ,  &find_data
            ,  FindExSearchNameMatch
            ,  NULL
            ,  get_windows7_dependent_value (FIND_FIRST_EX_LARGE_FETCH, 0)
            ))
   {
      
   }

   bool const find_file::is_valid () const throw ()
   {
      return find_file_handle != INVALID_HANDLE_VALUE;
   }

   bool const find_file::find_next () throw ()
   {
      return IMPLICIT_CAST (FindNextFile (find_file_handle, &find_data));
   }

   bool const find_file::is_directory () const throw ()
   {
      return (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
   }

   big_size const find_file::get_size () const throw ()
   {
      return (static_cast<big_size>(find_data.nFileSizeHigh) << 32) | find_data.nFileSizeLow;
   }

   FILETIME const find_file::get_creation_time () const throw ()
   {
      return find_data.ftCreationTime;
   }

   FILETIME const find_file::get_last_access_time () const throw ()
   {
      return find_data.ftLastAccessTime;
   }

   FILETIME const find_file::get_last_write_time () const throw ()
   {
      return find_data.ftLastWriteTime;
   }

   DWORD const find_file::get_reparse_point_tag () const throw ()
   {
      if (IS_ON (find_data.dwFileAttributes, FILE_ATTRIBUTE_REPARSE_POINT))
      {
         return find_data.dwReserved0;
      }
      else
      {
         return 0;
      }
   }

   DWORD const find_file::get_file_attributes () const throw ()
   {
      return find_data.dwFileAttributes;
   }

   LPCTSTR const find_file::get_name () const throw ()
   {
      if (is_valid ())
      {
         return find_data.cFileName ? find_data.cFileName : _T ("");
      }
      else
      {
         return _T ("");
      }
   }

   find_file::~find_file () throw ()
   {
      BOOL const close_result = FindClose (find_file_handle);
      UNUSED_VARIABLE (close_result);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   event::event (event_type::type const event_type)
      :  value (CreateEvent (NULL, event_type == event_type::manual_reset ? TRUE : FALSE, FALSE, NULL))
   {
   }

   void event::set () throw ()
   {
      if (value.is_valid ())
      {
         SetEvent (value.value);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   paint_device_context::paint_device_context (HWND const hwnd_) throw ()
      :  hwnd  (hwnd_)
      ,  hdc   (BeginPaint (hwnd_, const_cast<LPPAINTSTRUCT> (&paint_struct)))
   {
   }

   paint_device_context::~paint_device_context () throw ()
   {
      auto result = EndPaint (hwnd, &paint_struct);
      UNUSED_VARIABLE (result);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   window_device_context::window_device_context (HWND const hwnd_) throw ()
      :  hwnd  (hwnd_)
      ,  hdc   (GetWindowDC (hwnd_))
   {
   }

   window_device_context::~window_device_context () throw ()
   {
      auto result = ReleaseDC (hwnd, hdc);
      UNUSED_VARIABLE (result);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   device_context::device_context (HDC const dc) throw ()
      :  value (dc)
   {
   }

   device_context::~device_context () throw ()
   {
      auto deleted_result = DeleteDC (value);
      UNUSED_VARIABLE (deleted_result);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   select_object::select_object (HDC const dc_, HGDIOBJ const obj_) throw ()
      :  dc                         (dc_)
      ,  previously_selected_object (SelectObject (dc_, obj_))
   {
   }

   select_object::~select_object () throw ()
   {
      SelectObject (dc, previously_selected_object);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
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
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   gdi_object<HFONT> const create_font (LPCTSTR const font_family, int const height)
   {
      if (!font_family)
      {
         return gdi_object<HFONT> (NULL);
      }

      LOGFONT lf = {0};
      lf.lfHeight = height;
      _tcscpy_s (lf.lfFaceName, font_family);

      return gdi_object<HFONT> (CreateFontIndirect (&lf));
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   gdi_object<HFONT> const create_standard_font (
         standard_font::type const font_type
      ,  int const height
      )
   {
      NONCLIENTMETRICS non_client_metrics = {0};
      non_client_metrics.cbSize = sizeof (NONCLIENTMETRICS);

      auto system_parameters_info_result = SystemParametersInfo (
            SPI_GETNONCLIENTMETRICS
         ,  sizeof (NONCLIENTMETRICS)
         ,  &non_client_metrics
         ,  0);
      
      if (system_parameters_info_result)
      {
         LOGFONT lf = {0};

         switch (font_type)
         {
         case standard_font::caption:
            lf = non_client_metrics.lfCaptionFont;
         case standard_font::menu:
            lf = non_client_metrics.lfMenuFont;
         case standard_font::status:
            lf = non_client_metrics.lfStatusFont;
         case standard_font::message:
            lf = non_client_metrics.lfMessageFont;
            break;
         default:
            FS_ASSERT(false);
            lf = non_client_metrics.lfMessageFont;
            break;
         }

         if (height != 0)
         {
            lf.lfHeight = height;
         }

         return gdi_object<HFONT> (
            CreateFontIndirect (&lf));
      }
      else
      {
         return gdi_object<HFONT> (NULL);
      }

   }

   gdi_object<HFONT> const create_standard_font (standard_font::type const font_type)
   {
      return create_standard_font (font_type, 0);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   tstring const load_string_resource (int const resource_id)
   {
      return load_string_resource (resource_id, _T (""));
   }

   tstring const load_string_resource (int const resource_id, LPCTSTR const default_value)
   {
      LPCTSTR value = NULL;
      auto load_string_result = LoadString (
            GetModuleHandle (NULL)
         ,  resource_id
         ,  reinterpret_cast<LPTSTR> (&value)
         ,  0);

      if (load_string_result)
      {
         return tstring (value, load_string_result);
      }
      else if (default_value)
      {
         return tstring (default_value);
      }
      else
      {
         return tstring ();
      }
   }
   gdi_object<HBITMAP> const load_bitmap_resource (int resource_id)
   {
      return gdi_object<HBITMAP> (
         LoadBitmap (
               GetModuleHandle (NULL)
            ,  MAKEINTRESOURCE (resource_id)));
   }

   SIZE const get_bitmap_size (HDC const hdc, HBITMAP const bitmap)
   {
      SIZE result = {0};
      if (hdc && bitmap)
      {
         BITMAPINFO bmi = {0};
         bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
         auto get_bits_result = GetDIBits (
               hdc
            ,  bitmap
            ,  0
            ,  0
            ,  NULL
            ,  &bmi
            ,  DIB_RGB_COLORS
            );
         if (get_bits_result)
         {
            result.cx   = bmi.bmiHeader.biWidth    ;
            result.cy   = bmi.bmiHeader.biHeight   ;
         }
      }
      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   RECT const  zero_rect ()
   {
      RECT rect = {0};
      return rect;
   }

   bool const is_inside (RECT const & rect, POINT const & point) throw ()
   {
      return 
            rect.left < point.x && rect.right > point.x 
         && rect.top < point.y && rect.bottom > point.y
         ;
   }

   boost::optional<RECT> const intersect (RECT const & left, RECT const & right) throw ()
   {
      RECT result = {0};

      if (IntersectRect (
            &result
         ,  &left
         ,  &right
         ))
      {
         return result;
      }
      else
      {
         return boost::optional<RECT> ();
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   POINT const get_client_mouse_coordinate (HWND const hwnd, LPARAM const lParam)
   {
      auto p = get_mouse_coordinate (lParam);

      ScreenToClient (hwnd, &p);

      return p;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   POINT const get_mouse_coordinate (LPARAM const lParam)
   {
      POINT p = {0};

      p.x = static_cast<short> (LOWORD (lParam));
      p.y = static_cast<short> (HIWORD (lParam));

      return p;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
