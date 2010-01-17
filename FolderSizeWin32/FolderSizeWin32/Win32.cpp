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

   bool const thread::join (DWORD const ms) const throw ()
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
            path.c_str (),
            &find_data))
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
}
// ----------------------------------------------------------------------------
