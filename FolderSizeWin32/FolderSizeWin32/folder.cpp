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
#undef max
#undef min
// ----------------------------------------------------------------------------
#include "folder.hpp"
// ----------------------------------------------------------------------------
#include <winnt.h>
// ----------------------------------------------------------------------------
namespace folder
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace s    = std          ;
   namespace st   = s::tr1       ;
   namespace b    = boost        ;
   namespace f    = folder       ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::initializer::initializer (
         folder * const          parent_
      ,  tstring const &         name_
      ,  big_size const          size_
      ,  big_size const          physical_size_          
      ,  big_size const          file_count_
      ,  big_size const          folder_count_   
      ,  win32::file_time const  last_activity_
      ,  bool const              is_inaccessible_
      )
      :  parent                  (parent_       )
      ,  name                    (name_         )          
      ,  size                    (size_         )
      ,  physical_size           (physical_size_)
      ,  file_count              (file_count_   )
      ,  folder_count            (folder_count_ )
      ,  last_activity           (last_activity_)
      ,  is_inaccessible         (is_inaccessible_)
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder ()
      :  parent                           (nullptr             )

      ,  name                             (_T ("")             )

      ,  sub_folders                      (new f::folder * [0] )

      ,  size                             (0                   )
      ,  physical_size                    (0                   )
      ,  file_count                       (0                   )
      ,  folder_count                     (0                   )
      ,  last_activity                    (0                   )
      ,  is_inaccessible                  (0                   )
      ,  depth                            (0                   )
      ,  total_size                       (0                   )
      ,  total_physical_size              (0                   )
      ,  total_file_count                 (0                   )
      ,  total_folder_count               (0                   )
      ,  total_inaccessible_folder_count  (0                   )
   {
   }
   // -------------------------------------------------------------------------

   static int const initial_depth = 1;
   // -------------------------------------------------------------------------
   folder::folder (
         initializer const & init)
      :  parent                           (init.parent                                             )

      ,  name                             (init.name                                               )

      ,  sub_folders                      (new f::folder * [static_cast<int> (init.folder_count)]  )

      ,  size                             (init.size                                               )
      ,  physical_size                    (init.physical_size                                      )
      ,  file_count                       (init.file_count                                         )
      ,  folder_count                     (init.folder_count                                       )
      ,  last_activity                    (init.last_activity                                      )
      ,  is_inaccessible                  (init.is_inaccessible                                    )
      ,  depth                            (initial_depth                                           )
      ,  total_size                       (init.size                                               )
      ,  total_physical_size              (init.physical_size                                      )
      ,  total_file_count                 (init.file_count                                         )
      ,  total_folder_count               (init.folder_count                                       )
      ,  total_inaccessible_folder_count  (init.is_inaccessible ? 1 : 0                            )
   {
      memset (
            sub_folders.get ()
         ,  0
         ,  static_cast<std::size_t> (init.folder_count) * sizeof (void*));
      if (parent)
      {
         parent->recursive_update (
               initial_depth
            ,  size
            ,  physical_size
            ,  file_count
            ,  folder_count
            ,  is_inaccessible
            );
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   std::size_t const folder::get_depth () const throw ()
   {
      return depth.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_size () const throw ()
   {
      return total_size.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_physical_size () const throw ()
   {
      return total_physical_size.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_file_count () const throw ()
   {
      return total_file_count.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_folder_count () const throw ()
   {
      return total_folder_count.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_inaccessible_folder_count () const throw ()
   {
      return total_inaccessible_folder_count.get ();
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void folder::recursive_update  (
         std::size_t const    child_depth
      ,  big_size const       size         
      ,  big_size const       physical_size         
      ,  big_size const       file_count   
      ,  big_size const       folder_count 
      ,  bool const           folder_is_inaccessible        
      )
   {
      depth.inplace_max                            (child_depth + 1                 );
      total_size.inplace_add                       (size                            );
      total_physical_size.inplace_add              (physical_size                   );
      total_file_count.inplace_add                 (file_count                      );
      total_folder_count.inplace_add               (folder_count                    );
      total_inaccessible_folder_count.inplace_add  (folder_is_inaccessible ? 1 : 0  );

      if (parent)
      {
         parent->recursive_update (
               child_depth + 1
            ,  size
            ,  physical_size
            ,  file_count
            ,  folder_count
            ,  folder_is_inaccessible
            );
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder const folder::empty;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
