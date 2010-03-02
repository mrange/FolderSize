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
#include <functional>
#include <string>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../folder.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
namespace traverser
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   typedef std::tr1::function<
      void (folder::folder const *, std::size_t, std::size_t)
      > folder_state_changed_callback;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct traverser : boost::noncopyable
   {
      traverser (
            folder_state_changed_callback const &  folder_state_changed
         ,  win32::tstring const &                 path
         );
      ~traverser () throw ();

      win32::tstring const &  get_root_path () const throw ();

      folder::folder const *  get_root () const throw ();

      std::size_t const       get_processed_folder_count () const throw ();
      std::size_t const       get_unprocessed_folder_count () const throw ();

      void stop_traversing () throw ();

   private:
      struct impl;

      std::auto_ptr<impl> const m_impl;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
