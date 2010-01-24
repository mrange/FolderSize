/* ****************************************************************************
 *
 * Copyright (c) M�rten R�nge.
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
   handle::handle (HANDLE hnd) throw ()
      :  value (hnd)
   {
   }

   handle::~handle () throw ()
   {
      if (is_valid ())
      {
         BOOL const close_result = CloseHandle (value);
      }
   }

   bool const handle::is_valid () const throw ()
   {
      return value != INVALID_HANDLE_VALUE;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   thread::thread (proc const del)
      :  procedure (del)
      ,  value (reinterpret_cast<HANDLE> (_beginthread (raw_proc, 0, this)))
      ,  terminated (false)
   {
   }

   bool const thread::join (unsigned int const ms) const throw ()
   {
      if (value.is_valid ())
      {
         auto res = WaitForSingleObject (value.value, ms);

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

   void thread::raw_proc (void * ptr) throw ()
   {
      auto state = static_cast<thread *> (ptr);

      if (state)
      {
         try
         {
            auto result = state->procedure ();
            state->terminated = true;
            _endthreadex (result);
         }
         catch(...)
         {
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
      tstring const & path)
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
      return FindNextFile (find_file_handle, &find_data);
   }

   bool const find_file::is_directory () const throw ()
   {
      return (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
   }

   __int64 const find_file::get_size () const throw ()
   {
      return find_data.nFileSizeHigh << 32 | find_data.nFileSizeLow;
   }

   LPCTSTR const find_file::get_name () const throw ()
   {
      if (is_valid ())
      {
         return find_data.cFileName ? find_data.cFileName : _T("");
      }
      else
      {
         return _T("");
      }
   }

   find_file::~find_file () throw ()
   {
      BOOL const close_result = FindClose (find_file_handle);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   event::event (bool auto_reset)
      :  value (CreateEvent (NULL, auto_reset, FALSE, NULL))
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
   device_context::device_context (HDC const dc) throw ()
      :  value (dc)
   {
   }

   device_context::~device_context () throw ()
   {
      auto deleted_result = DeleteDC (value);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   gdi_object::gdi_object (HGDIOBJ const obj) throw ()
      :  value (obj)
   {
   }

   gdi_object::~gdi_object () throw ()
   {
      auto deleted_result = DeleteObject (value);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   select_object::select_object (HDC const dc_, HGDIOBJ obj_) throw ()
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
}
// ----------------------------------------------------------------------------
