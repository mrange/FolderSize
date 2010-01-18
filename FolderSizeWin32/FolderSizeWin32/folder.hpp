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
#pragma once
// ----------------------------------------------------------------------------
#include <tchar.h>
// ----------------------------------------------------------------------------
#include <string>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
// ----------------------------------------------------------------------------
namespace folder
{
   // -------------------------------------------------------------------------
   typedef std::basic_string<TCHAR> tstring;
   // -------------------------------------------------------------------------
   struct folder : boost::noncopyable
   {
      typedef boost::scoped_array<folder const *> folder_array;

      struct initializer
      {
         tstring const &         name           ;
         unsigned __int64 const  size           ;
         std::size_t             file_count     ;
         std::size_t const       folder_count   ;

         initializer (
            tstring const &         name_          ,
            unsigned __int64 const  size_          ,
            std::size_t             file_count_    ,
            std::size_t const       folder_count_   
            );
      };

      folder ();

      folder (
         initializer const & init);

      tstring const           name;

      unsigned __int64 const  size;
      std::size_t const       file_count;
      std::size_t const       folder_count;
      folder_array const      sub_folders;

      static folder const  empty;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
