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
// ----------------------------------------------------------------------------
#include "../folder.hpp"
#include "../win32.hpp"
// ----------------------------------------------------------------------------
namespace traverser
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct traverser : boost::noncopyable
   {
      traverser (
            HWND const main_hwnd
         ,  win32::tstring const & path);
      ~traverser () throw ();

      folder::folder const * get_root () const throw ();

      void stop_traversing () throw ();

   private:
      struct impl;

      std::auto_ptr<impl> const m_impl;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
