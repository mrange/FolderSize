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
// ----------------------------------------------------------------------------
#include <algorithm>
#include <deque>
#include <memory>
#include <string>
#include <vector>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
// ----------------------------------------------------------------------------
#include "Traverser.hpp"
// ----------------------------------------------------------------------------
namespace traverser
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace s    = std          ;
   namespace st   = s::tr1       ;
   namespace b    = boost        ;
   namespace f    = folder       ;
   namespace w    = win32        ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct handle : b::noncopyable
   {
      handle (HANDLE hnd) throw ()
         :  value (hnd)
      {
      }

      ~handle () throw ()
      {
         if (is_valid ())
         {
            BOOL const close_result = CloseHandle (value);
         }
      }

      bool const is_valid () const throw ()
      {
         return value != INVALID_HANDLE_VALUE;
      }

      HANDLE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct thread : b::noncopyable
   {
      typedef st::function<unsigned int ()> proc;
      thread (proc const del)
         :  procedure (del)
         ,  value (reinterpret_cast<HANDLE> (_beginthread (raw_proc, 0, this)))
         ,  terminated (false)
      {
      }

      proc const        procedure;
      handle const      value;

      bool const join (DWORD const ms) const throw ()
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
   
      bool is_terminated () const throw ()
      {
         return terminated;
      }

      static void raw_proc (void * ptr) throw ()
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

      bool volatile        terminated;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct find_file : b::noncopyable
   {
      find_file (
         w::tstring const & path)
         :  find_file_handle (FindFirstFile (
               path.c_str (),
               &find_data))
      {
         
      }

      bool const is_valid () const throw ()
      {
         return find_file_handle != INVALID_HANDLE_VALUE;
      }

      bool const find_next () throw ()
      {
         return FindNextFile (find_file_handle, &find_data);
      }

      bool const is_directory () const throw ()
      {
         return (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
      }

      __int64 const get_size () const throw ()
      {
         return find_data.nFileSizeHigh << 32 | find_data.nFileSizeLow;
      }

      LPCTSTR const get_name () const throw ()
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

      ~find_file () throw ()
      {
         BOOL const close_result = FindClose (find_file_handle);
      }

   private:
      WIN32_FIND_DATA   find_data;
      HANDLE const      find_file_handle;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct traverser::impl : b::noncopyable
   {
      struct job
      {
         job (
            w::tstring const & p,
            w::tstring const & n,
            f::folder const ** fr)
            :  path (p)
            ,  name (n)
            ,  folder_replacement (fr)
         {
         }

         w::tstring           path                 ;
         w::tstring           name                 ;
         f::folder const **   folder_replacement   ;
      };

      b::object_pool<f::folder>  folder_pool       ;
      s::deque<job>              job_queue         ;
      w::tstring const           root_path         ;
      f::folder const *          root              ;
      bool volatile              continue_running  ;
      thread const               thread            ;     

      s::deque<job> const     create_initial_queue (job const & initial_job)
      {
         s::deque<job> queue;
         queue.push_back (initial_job);
         return queue;
      }

      impl (w::tstring const & path)
         :  job_queue         (create_initial_queue (job (path, _T("."), &root)))
         ,  root_path         (path)
         ,  root              (folder_pool.construct ())
         ,  continue_running  (true)
         ,  thread            (create_proc ())
      {
      }

      ~impl ()
      {
         continue_running = false;

         thread.join (1000);
      }

      unsigned int proc()
      {
         while (continue_running && job_queue.size () > 0)
         {
            auto current_job = job_queue.front ();

            find_file ff (current_job.path + _T("\\*.*"));

            if (continue_running && ff.is_valid ())
            {
               s::vector <w::tstring>  folder_names      ;
               __int64                 size           (0);
               __int64                 file_count     (0);

               do
               {
                  w::tstring const name = ff.get_name ();

                  if (ff.is_directory ())
                  {
                     if (name != _T(".") && name != _T(".."))
                     {
                        folder_names.push_back (ff.get_name ());
                     }
                  }
                  else
                  {
                     size += ff.get_size ();
                     ++file_count;
                  }
               }
               while (continue_running && ff.find_next ());

               auto folder_count = folder_names.size ();

               auto new_folder = folder_pool.construct (
                  f::folder::initializer (
                        current_job.name
                     ,  size
                     ,  file_count
                     ,  folder_count));

               for (s::size_t iter = 0; iter < folder_count; ++iter)
               {
                  auto sub_folder_ref = new_folder->sub_folders.get () + iter;
                  auto folder_name = folder_names[iter];

                  job_queue.push_back (
                     job (
                        current_job.path + _T("\\") + folder_name,
                        folder_name,
                        sub_folder_ref
                        ));
               }

               if (current_job.folder_replacement)
               {
                  *(current_job.folder_replacement) = new_folder;
               }
            }

            job_queue.pop_front ();
         }


         return EXIT_SUCCESS;
      }

      thread::proc create_proc () throw ()
      {
         return st::bind (&impl::proc, this);
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   traverser::traverser (w::tstring const & path)
      :  m_impl (new impl (path))
   {
   }

   f::folder const * traverser::get_root () const throw ()
   {
      return m_impl->root;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
