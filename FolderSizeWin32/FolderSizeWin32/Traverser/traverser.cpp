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
#include <boost/optional.hpp>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
#include "traverser.hpp"
// ----------------------------------------------------------------------------
#undef max
#undef min
// ----------------------------------------------------------------------------
namespace traverser
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace b    = boost        ;
   namespace f    = folder       ;
   namespace s    = std          ;
   namespace w    = win32        ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct traverser::impl : b::noncopyable
   {
      struct job
      {
         job (
               w::tstring const &         path_
            ,  w::tstring const &         name_
            ,  f::folder * const          parent_folder_
            ,  f::folder ** const         replacement_folder_
            )
            :  path                 (path_               )
            ,  name                 (name_               )
            ,  parent_folder        (parent_folder_      )
            ,  replacement_folder   (replacement_folder_ )
         {
         }

         w::tstring           path                 ;
         w::tstring           name                 ;
         f::folder *          parent_folder        ;
         f::folder **         replacement_folder   ;
      };

      folder_state_changed_callback const  folder_state_changed       ;
      b::object_pool<f::folder>            folder_pool                ;
      s::deque<job>                        job_queue                  ;
      w::tstring const                     root_path                  ;
      f::folder *                          root                       ;
      bool volatile                        continue_running           ;
      w::thread const                      thread                     ;
      std::size_t volatile                 processed_folder_count     ;
      std::size_t volatile                 unprocessed_folder_count   ;

      boost::optional<DWORD>               send_next_update           ;

      s::deque<job> const     create_initial_queue (job const & initial_job)
      {
         s::deque<job> queue;
         queue.push_back (initial_job);
         return queue;
      }

      impl (
            folder_state_changed_callback const &  folder_state_changed_
         ,  w::tstring const &                     path
         )
         :  folder_state_changed       (folder_state_changed_                                      )
         ,  job_queue                  (create_initial_queue (job (path, _T ("."), nullptr, &root)))
         ,  root_path                  (path                                                       )
         ,  root                       (folder_pool.construct ()                                   )
         ,  continue_running           (true                                                       )
         ,  thread                     (_T ("traverser"), create_proc ()                           )
         ,  processed_folder_count     (0                                                          )
         ,  unprocessed_folder_count   (0                                                          )
      {
      }

      ~impl ()
      {
         continue_running = false;

         thread.join (10000);
      }

      void update_view (
            folder::folder const *  root_folder
         ,  std::size_t const       unprocessed_folder_count
         ,  std::size_t const       processed_folder_count
         ,  bool const              force_update = false
         )
      {
         auto tick_count = GetTickCount ();

         if (
               force_update
            || !send_next_update
            || *send_next_update < tick_count)
         {
            send_next_update = tick_count + 40; //20ms delay

            if (folder_state_changed)
            {
               folder_state_changed (
                     root_folder
                  ,  unprocessed_folder_count
                  ,  processed_folder_count
                  );
            }
         }
      }

      bool const skippable_reparse_point (DWORD const reparse_tag) noexcept
      {
         if (
               reparse_tag == IO_REPARSE_TAG_SYMLINK
            || reparse_tag == IO_REPARSE_TAG_MOUNT_POINT)
         {
            return true;
         }
         else
         {
            return false;
         }
      }

      unsigned int proc ()
      {
         while (continue_running && !job_queue.empty ())
         {
            auto current_job = job_queue.front ();

            w::find_file find_file (current_job.path + _T ("\\*.*"));

            if (!find_file.is_valid ())
            {
               auto new_folder = folder_pool.construct (
                  f::folder::initializer (
                        current_job.parent_folder
                     ,  current_job.name
                     ,  0
                     ,  0
                     ,  0
                     ,  0
                     ,  w::to_file_time (w::get_current_time ())
                     ,  true
                     ));

               if (current_job.replacement_folder)
               {
                  *(current_job.replacement_folder) = new_folder;
               }
            }
            else if (continue_running)
            {
               s::vector <
                     w::tstring
                  ,  b::pool_allocator<w::tstring>>
                                       folder_names      ;
               big_size                size           (0);
               big_size                physical_size  (0);
               std::size_t             file_count     (0);
               w::file_time            last_activity  (0);

               folder_names.reserve (1024);


               do
               {
                  w::tstring const name   = find_file.get_name ();

                  auto creation_time      = w::to_file_time (find_file.get_creation_time ());
                  auto last_write_time    = w::to_file_time (find_file.get_creation_time ());

                  last_activity           = std::max (
                        last_activity
                     ,  std::max (creation_time, last_write_time)
                     );


                  if (find_file.is_directory ())
                  {
                     if (
                           !skippable_reparse_point (find_file.get_reparse_point_tag ())
                        && name != _T (".")
                        && name != _T (".."))
                     {
                        folder_names.push_back (find_file.get_name ());
                     }
                  }
                  else
                  {
                     if (!skippable_reparse_point (find_file.get_reparse_point_tag ()))
                     {
                        size += find_file.get_size ();
                        if (ANY_IS_ON (
                              find_file.get_file_attributes ()
                           ,  FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_SPARSE_FILE
                           ))
                        {
                           auto file_path = current_job.path + _T ("\\") + find_file.get_name ();
                           DWORD hiword = 0;
                           DWORD loword = GetCompressedFileSize (
                                 file_path.c_str ()
                              ,  &hiword);

                           if (loword != INVALID_FILE_SIZE)
                           {
                              physical_size += static_cast<big_size> (hiword) << 32 | loword;
                           }
                        }
                        else
                        {
                           // Potentially we should
                           physical_size += find_file.get_size ();
                        }
                     }

                     ++file_count;
                  }
               }
               while (continue_running && find_file.find_next ());

               auto folder_count = folder_names.size ();

               auto new_folder = folder_pool.construct (
                  f::folder::initializer (
                        current_job.parent_folder
                     ,  current_job.name
                     ,  size
                     ,  physical_size
                     ,  file_count
                     ,  folder_count
                     ,  last_activity
                     ,  false
                     ));

               for (s::size_t iter = 0; continue_running && iter < folder_count; ++iter)
               {
                  auto sub_folder_ref = new_folder->sub_folders.get () + iter;
                  auto folder_name = folder_names[iter];

                  job_queue.push_back (
                     job (
                           current_job.path + _T ("\\") + folder_name
                        ,  folder_name
                        ,  new_folder
                        ,  sub_folder_ref
                        ));
               }

               if (current_job.replacement_folder)
               {
                  *(current_job.replacement_folder) = new_folder;
               }

               update_view (
                     root
                  ,  unprocessed_folder_count
                  ,  processed_folder_count
                  );

            }

            job_queue.pop_front ();

            unprocessed_folder_count   =  job_queue.size ();
            processed_folder_count     =  processed_folder_count + 1;
         }

         if (continue_running)
         {
            update_view (
                  root
               ,  unprocessed_folder_count
               ,  processed_folder_count
               ,  true
               );
         }

         return EXIT_SUCCESS;
      }

      w::thread::proc create_proc () noexcept
      {
         return s::bind (&impl::proc, this);
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   traverser::traverser (
         folder_state_changed_callback const &  folder_state_changed
      ,  w::tstring const &                     path
      )
      :  m_impl (new impl (folder_state_changed, path))
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   traverser::~traverser () noexcept
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void traverser::stop_traversing () noexcept
   {
      m_impl->continue_running = false;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   w::tstring const & traverser::get_root_path () const noexcept
   {
      return m_impl->root_path;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   f::folder const * traverser::get_root () const noexcept
   {
      return m_impl->root;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   std::size_t const traverser::get_processed_folder_count () const noexcept
   {
      return m_impl->processed_folder_count;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   std::size_t const traverser::get_unprocessed_folder_count () const noexcept
   {
      return m_impl->unprocessed_folder_count;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
