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
         tstring const &         name_
      ,  unsigned __int64 const  size_
      ,  std::size_t             file_count_
      ,  std::size_t const       folder_count_   
      )
      :  name           (name_)          
      ,  size           (size_)
      ,  file_count     (file_count_)
      ,  folder_count   (folder_count_)
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder ()
      :  name           (_T(""))
      ,  size           (0)
      ,  file_count     (0)
      ,  folder_count   (0)
      ,  sub_folders    (new f::folder const * [0])
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder::folder (
         initializer const & init)
      :  name           (init.name)
      ,  size           (init.size)
      ,  file_count     (init.file_count)
      ,  folder_count   (init.folder_count)
      ,  sub_folders    (new f::folder const * [init.folder_count])
   {
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   folder const folder::empty;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
