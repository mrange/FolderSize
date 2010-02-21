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
#include "folder.hpp"
// ----------------------------------------------------------------------------
#include <winnt.h>
// ----------------------------------------------------------------------------
#undef max
#undef min
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
      ,  big_size const          size_
      ,  big_size const          physical_size_          
      ,  std::size_t const       file_count_
      ,  std::size_t const       folder_count_   
      )
      :  parent         (parent_       )
      ,  name           (name_         )          
      ,  size           (size_         )
      ,  physical_size  (physical_size_)
      ,  file_count     (file_count_   )
      ,  folder_count   (folder_count_ )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder ()
      :  parent               (NULL                      )
      ,  name                 (_T ("")                   )
      ,  size                 (0                         )
      ,  physical_size        (0                         )
      ,  file_count           (0                         )
      ,  folder_count         (0                         )
      ,  sub_folders          (new f::folder * [0]       )
      ,  depth                (0                         )
      ,  total_size           (0                         )
      ,  total_physical_size  (0                         )
      ,  total_file_count     (0                         )
      ,  total_folder_count   (0                         )
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder (
         initializer const & init)
      :  parent               (init.parent                                             )
      ,  name                 (init.name                                               )
      ,  size                 (init.size                                               )
      ,  physical_size        (init.physical_size                                      )
      ,  file_count           (init.file_count                                         )
      ,  folder_count         (init.folder_count                                       )
      ,  sub_folders          (new f::folder * [static_cast<int> (init.folder_count)]  )
      ,  depth                (1                                                       )
      ,  total_size           (init.size                                               )
      ,  total_physical_size  (init.physical_size                                      )
      ,  total_file_count     (init.file_count                                         )
      ,  total_folder_count   (init.folder_count                                       )
   {
      memset (
            sub_folders.get ()
         ,  0
         ,  static_cast<std::size_t> (init.folder_count) * sizeof (void*));
      if (parent)
      {
         parent->recursive_update (
               depth
            ,  size
            ,  physical_size
            ,  file_count
            ,  folder_count);
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

      big_size const interlocked_add (big_size volatile* value, big_size const add)
      {
         auto value_ptr          = reinterpret_cast<__int64 volatile*>(value);
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

         return static_cast<big_size> (new_value);
      }

      std::size_t const interlocked_max (std::size_t volatile* value, std::size_t const max_value)
      {
         auto value_ptr          = reinterpret_cast<long volatile*>(value);
         long current_value      = 0;
         long new_value          = 0;
         do
         {
            current_value = *value_ptr;
            new_value = static_cast<long> (std::max (static_cast<std::size_t> (current_value), max_value));
         }
         while (
            current_value != _InterlockedCompareExchange (
               value_ptr
            ,  new_value
            ,  current_value
            ));

         return static_cast<std::size_t> (new_value);
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   std::size_t const folder::get_depth () const throw ()
   {
      return depth;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_size () const throw ()
   {
      return interlocked_read (&total_size);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   big_size const folder::get_total_physical_size () const throw ()
   {
      return interlocked_read (&total_physical_size);
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
         std::size_t const child_depth
      ,  big_size const size         
      ,  big_size const physical_size         
      ,  big_size const file_count   
      ,  big_size const folder_count 
      )
   {
      interlocked_max (&depth                , child_depth + 1 );
      interlocked_add (&total_size           , size            );
      interlocked_add (&total_physical_size  , physical_size   );
      interlocked_add (&total_file_count     , file_count      );
      interlocked_add (&total_folder_count   , folder_count    );

      if (parent)
      {
         parent->recursive_update (
               child_depth + 1
            ,  size
            ,  physical_size
            , file_count
            , folder_count
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
