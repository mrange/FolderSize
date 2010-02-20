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
   void output_debug_string (tstring const & value)
   {
      output_debug_string (value.c_str ());
   }

   void output_debug_string (LPCTSTR const value)
   {
#ifdef _DEBUG
      if (value)
      {
         OutputDebugString (value);
      }
      OutputDebugString (_T ("\r\n"));
#endif
   }

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
         output_debug_string (_T ("FolderSize.Win32 : Joining thread: ") + thread_name);
         auto res = WaitForSingleObject (value.value, ms);

         if (res == WAIT_OBJECT_0)
         {
            output_debug_string (_T ("FolderSize.Win32 : Thread join successful: ") + thread_name);
         }
         else
         {
            output_debug_string (_T ("FolderSize.Win32 : Thread join failed: ") + thread_name);
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
            output_debug_string (_T ("FolderSize.Win32 : Starting thread: ") + state->thread_name);

            auto result = state->procedure ();


            if (result == EXIT_SUCCESS)
            {
               output_debug_string (_T ("FolderSize.Win32 : Thread exited successfully: ") + state->thread_name);
            }
            else
            {
               output_debug_string (_T ("FolderSize.Win32 : Thread exited with failure code: ") + state->thread_name);
            }

            state->terminated = true;
            _endthreadex (result);
         }
         catch (...)
         {
            output_debug_string (_T ("FolderSize.Win32 : Thread threw exception: ") + state->thread_name);
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
      :  find_file_handle (FindFirstFile (
               path.c_str ()
            ,  &find_data))
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
   namespace
   {
      XFORM const get_world_transform (HDC const dc_) throw ()
      {
         XFORM xform = {0};
         // Intentionally ignores result
         GetWorldTransform (dc_, &xform);
         return xform;
      }
   }

   set_world_transform::set_world_transform (HDC const dc_, XFORM const * const transform) throw ()
      :  dc (dc_)
      ,  old_transform (get_world_transform (dc_))
   {
      // Intentionally ignores result
      SetWorldTransform (dc_, transform);
   }

   set_world_transform::~set_world_transform () throw ()
   {
      SetWorldTransform (dc, &old_transform);
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
   gdi_object<HFONT> const create_standard_message_font (int const height)
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
         if (height != 0)
         {
            non_client_metrics.lfMessageFont.lfHeight = height;
         }
         return gdi_object<HFONT> (
            CreateFontIndirect (
               &non_client_metrics.lfMessageFont));
      }
      else
      {
         return gdi_object<HFONT> (NULL);
      }

   }

   gdi_object<HFONT> const create_standard_message_font ()
   {
      return create_standard_message_font (0);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   gdi_object<HBITMAP> const load_bitmap_resource (int resource_id)
   {
      return gdi_object<HBITMAP> (
         LoadBitmap (
               GetModuleHandle (NULL)
            ,  MAKEINTRESOURCE (resource_id)));
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   bool const is_inside (RECT const & rect, POINT const & point)
   {
      return 
            rect.left < point.x && rect.right > point.x 
         && rect.top < point.y && rect.bottom > point.y
         ;
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
