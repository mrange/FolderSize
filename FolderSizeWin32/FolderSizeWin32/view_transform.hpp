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
#include "linear.hpp"
// ----------------------------------------------------------------------------
namespace view_transform
{
   typedef linear::matrix<double, 3, 3>   transform;
   typedef linear::vector<double, 2>      vector;
   typedef linear::vector<double, 2>      dimension;
   typedef linear::vector<double, 3>      extended_vector;

   extended_vector const create_extended_vector (double const x, double const y);
   vector const create_vector (double const x, double const y);

   namespace transform_direction
   {
      enum type
      {
         forward  ,
         reverse  ,
      };
   }

   transform const view_to_screen (
         transform_direction::type const direction
      ,  dimension const & size
      ,  vector const & centre
      ,  vector const & zoom
      );

   transform const bitmap_to_screen_transform (
         transform_direction::type const direction
      ,  dimension const & size
      ,  vector const & centre
      ,  vector const & zoom
      );
}
// ----------------------------------------------------------------------------
   