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
#include "map"
// ----------------------------------------------------------------------------
#include "../../FolderSizeWin32/FolderSizeWin32/folder.hpp"
#include "../../FolderSizeWin32/FolderSizeWin32/Traverser/traverser.hpp"
// ----------------------------------------------------------------------------
namespace
{
   // -------------------------------------------------------------------------
   namespace f = folder ;
   namespace s = std    ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void print_status (f::folder const *, s::size_t unproc, s::size_t proc)
   {
      _ftprintf_s (stderr, _T ("Unprocessed %d, Processed %d\r\n"), unproc, proc);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   s::size_t const recursive_print (
         s::size_t const next_id
      ,  s::size_t const parent_id
      ,  f::folder const * f)
   {
      if (f)
      {
         auto this_id = next_id;
         auto sub_next_id = next_id + 1;
         _tprintf_s (
               _T ("%4d|%4d|%40s|%4d|%6I64d|%6I64d|%10I64d|%10I64d|%8I64d|%8I64d|%12I64d|%12I64d\r\n")
            ,  this_id
            ,  parent_id
            ,  f->name.c_str ()
            ,  f->get_depth ()
            ,  f->folder_count
            ,  f->file_count
            ,  f->size
            ,  f->physical_size
            ,  f->get_total_folder_count ()
            ,  f->get_total_file_count ()
            ,  f->get_total_size ()
            ,  f->get_total_physical_size ()
            );

         for (big_size iter = 0; iter < f->folder_count; ++iter)
         {
            sub_next_id = recursive_print (
                  sub_next_id
               ,  this_id
               ,  f->sub_folders[static_cast<s::size_t> (iter)]
               );
         }

         return sub_next_id;
      }
      else
      {
         return next_id;
      }
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
int _tmain (int argc, _TCHAR* argv[])
{

   {
      if (argc != 2)
      {
         return EXIT_FAILURE;
      }

      traverser::traverser t (print_status, argv[1]);

      do
      {
         Sleep (100);
      }
      while (t.get_unprocessed_folder_count () > 0);


      _tprintf_s (
         _T ("  id| pid|                                    name|dep.|folder| files|      size|phys. size|t.folder| t.files|      t.size|t.phys. size\r\n")
         );
      recursive_print (1, 0, t.get_root ());

   }




	return EXIT_SUCCESS;
}
// ----------------------------------------------------------------------------
