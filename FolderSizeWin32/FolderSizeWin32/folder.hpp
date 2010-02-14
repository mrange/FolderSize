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
      typedef boost::scoped_array<folder *> folder_array;

      struct initializer
      {
         folder * const          parent         ;
         tstring const &         name           ;
         big_size const          size           ;
         big_size const          file_count     ;
         big_size const          folder_count   ;

         initializer (
               folder * const          parent
            ,  tstring const &         name          
            ,  big_size const          size          
            ,  std::size_t const       file_count
            ,  std::size_t const       folder_count
            );
      };

      folder ();

      explicit folder (
         initializer const & init
         );

      folder * const          parent;

      tstring const           name;

      folder_array const      sub_folders;

      big_size const          size;
      big_size const          file_count;
      big_size const          folder_count;

      big_size const          get_total_size () const throw ();
      big_size const          get_total_file_count () const throw ();
      big_size const          get_total_folder_count () const throw ();

      static folder const  empty;


   private:
      void                    recursive_update  (
                                    big_size const size         
                                 ,  big_size const file_count   
                                 ,  big_size const folder_count 
                                 );
      big_size volatile       total_size           ;
      big_size volatile       total_file_count     ;
      big_size volatile       total_folder_count   ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
