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
#include "../messages.hpp"
// ----------------------------------------------------------------------------
#include "traverser.hpp"
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
   struct traverser::impl : b::noncopyable
   {
      struct job
      {
         job (
               w::tstring const & path_
            ,  w::tstring const & name_
            ,  f::folder const ** folder_replacement_)
            :  path                 (path_               )
            ,  name                 (name_               )
            ,  folder_replacement   (folder_replacement_ )
         {
         }

         w::tstring           path                 ;
         w::tstring           name                 ;
         f::folder const **   folder_replacement   ;
      };

      HWND                       main_hwnd         ;
      b::object_pool<f::folder>  folder_pool       ;
      s::deque<job>              job_queue         ;
      w::tstring const           root_path         ;
      f::folder const *          root              ;
      bool volatile              continue_running  ;
      w::thread const            thread            ;     

      s::deque<job> const     create_initial_queue (job const & initial_job)
      {
         s::deque<job> queue;
         queue.push_back (initial_job);
         return queue;
      }

      impl (
            HWND const main_hwnd_
         ,  w::tstring const & path
         )
         :  main_hwnd         (main_hwnd_)
         ,  job_queue         (create_initial_queue (job (path, _T("."), &root)))
         ,  root_path         (path)
         ,  root              (folder_pool.construct ())
         ,  continue_running  (true)
         ,  thread            (_T("traverser"), create_proc ())
      {
      }

      ~impl ()
      {
         continue_running = false;

         thread.join (10000);
      }

      void update_view () const
      {
         PostMessage (
               main_hwnd
            ,  messages::folder_structure_changed
            ,  0
            ,  0);
      }

      unsigned int proc()
      {
         while (continue_running && job_queue.size () > 0)
         {
            auto current_job = job_queue.front ();

            w::find_file find_file (current_job.path + _T("\\*.*"));

            if (continue_running && find_file.is_valid ())
            {
               s::vector <
                     w::tstring
                  ,  b::pool_allocator<w::tstring>>  
                                       folder_names      ;
               unsigned __int64        size           (0);
               std::size_t             file_count     (0);

               folder_names.reserve (1024);

               do
               {
                  w::tstring const name = find_file.get_name ();

                  if (find_file.is_directory ())
                  {
                     if (name != _T(".") && name != _T(".."))
                     {
                        folder_names.push_back (find_file.get_name ());
                     }
                  }
                  else
                  {
                     size += find_file.get_size ();
                     ++file_count;
                  }
               }
               while (continue_running && find_file.find_next ());

               auto folder_count = folder_names.size ();

               auto new_folder = folder_pool.construct (
                  f::folder::initializer (
                        current_job.name
                     ,  size
                     ,  file_count
                     ,  folder_count));

               for (s::size_t iter = 0; continue_running && iter < folder_count; ++iter)
               {
                  auto sub_folder_ref = new_folder->sub_folders.get () + iter;
                  auto folder_name = folder_names[iter];

                  job_queue.push_back (
                     job (
                           current_job.path + _T("\\") + folder_name
                        ,  folder_name
                        ,  sub_folder_ref
                        ));
               }

               if (current_job.folder_replacement)
               {
                  *(current_job.folder_replacement) = new_folder;
               }

               update_view ();

            }

            job_queue.pop_front ();
         }

         update_view ();

         return EXIT_SUCCESS;
      }

      w::thread::proc create_proc () throw ()
      {
         return st::bind (&impl::proc, this);
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   traverser::traverser (
         HWND const main_hwnd
      ,  w::tstring const & path
      )
      :  m_impl (new impl (main_hwnd, path))
   {
   }

   traverser::~traverser () throw ()
   {
   }

   void traverser::stop_traversing () throw ()
   {
      m_impl->continue_running = false;
   }

   f::folder const * traverser::get_root () const throw ()
   {
      return m_impl->root;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
