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
   namespace
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::initializer::initializer (
         folder * const          parent_
      ,  tstring const &         name_
      ,  big_size const  size_
      ,  std::size_t             file_count_
      ,  std::size_t const       folder_count_   
      )
      :  parent         (parent_       )
      ,  name           (name_         )          
      ,  size           (size_         )
      ,  file_count     (file_count_   )
      ,  folder_count   (folder_count_ )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder ()
      :  parent               (NULL                      )
      ,  name                 (_T("")                    )
      ,  size                 (0                         )
      ,  file_count           (0                         )
      ,  folder_count         (0                         )
      ,  sub_folders          (new f::folder * [0]       )
      ,  total_size           (0                         )
      ,  total_file_count     (0                         )
      ,  total_folder_count   (0                         )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder (
         initializer const & init)
      :  parent               (init.parent                              )
      ,  name                 (init.name                                )
      ,  size                 (init.size                                )
      ,  file_count           (init.file_count                          )
      ,  folder_count         (init.folder_count                        )
      ,  sub_folders          (new f::folder * [init.folder_count]      )
      ,  total_size           (init.size                                )
      ,  total_file_count     (init.file_count                          )
      ,  total_folder_count   (init.folder_count                        )
   {
      memset (sub_folders.get (), 0, init.folder_count * sizeof (void*));
      if (parent)
      {
         parent->recursive_update (size, file_count, folder_count);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   namespace
   {
      big_size const interlocked_read (big_size const volatile* value)
      {
         return _InterlockedCompareExchange64 (
               reinterpret_cast<__int64 volatile*>(const_cast<big_size volatile*>(value))
            ,  0
            ,  0
            );
      }
      big_size const interlocked_add (big_size const volatile* value, big_size const add)
      {
         auto value_ptr          = reinterpret_cast<__int64 volatile*>(const_cast<big_size volatile*>(value));
         __int64 current_value  = 0;
         __int64 new_value      = 0;

         do
         {
            current_value = _InterlockedCompareExchange64 (
               value_ptr
            ,  0
            ,  0
            );
            new_value = static_cast<__int64> (static_cast<big_size> (current_value) + add);
         }
         while (
            current_value != _InterlockedCompareExchange64 (
               value_ptr
            ,  new_value
            ,  current_value
            ));

         return new_value;
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_size () const throw ()
   {
      return interlocked_read (&total_size);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_file_count () const throw ()
   {
      return interlocked_read (&total_file_count);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_folder_count () const throw ()
   {
      return interlocked_read (&total_folder_count);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void folder::recursive_update  (
         big_size const size         
      ,  big_size const file_count   
      ,  big_size const folder_count 
      )
   {
      interlocked_add (&total_size           , size         );
      interlocked_add (&total_file_count     , file_count   );
      interlocked_add (&total_folder_count   , folder_count );

      if (parent)
      {
         parent->recursive_update (size, file_count, folder_count);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder const folder::empty;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
