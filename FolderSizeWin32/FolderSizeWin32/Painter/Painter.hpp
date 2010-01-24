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
#include <windows.h>
// ----------------------------------------------------------------------------
#include <cstddef>
#include <functional>
#include <memory>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../Linear.hpp"
#include "../folder.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------
   typedef linear::vector<double, 2>      coordinate  ;
   typedef linear::vector<double, 2>      zoom_factor ;
   typedef linear::vector<double, 2>      dimension   ;
   // -------------------------------------------------------------------------
   struct painter : boost::noncopyable
   {
      struct impl;

      void paint (
            HDC const hdc
         ,  coordinate const & centre
         ,  zoom_factor const & zoom
         ,  dimension const & screen_size);

   private:
      std::auto_ptr<impl> m_impl;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
