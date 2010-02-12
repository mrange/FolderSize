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

// ----------------------------------------------------------------------------
#define UNUSED_VARIABLE(expr) expr
#define IMPLICIT_CAST(expr) (utility::implicit_cast (expr))
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
namespace utility
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TCastToType, typename TValueType>
   struct explicit_cast_helper;

   template<>
   struct explicit_cast_helper<int ,double>
   {
      inline static int const cast (double const &value) throw ()
      {
         return static_cast<int> (value + 0.5);
      }
   };

   template<>
   struct explicit_cast_helper<LONG, double>
   {
      inline static LONG const cast (double const &value) throw ()
      {
         return static_cast<LONG> (value + 0.5);
      }
   };

   template<>
   struct explicit_cast_helper<FLOAT, double>
   {
      inline static FLOAT const cast (double const &value) throw ()
      {
         return static_cast<FLOAT> (value);
      }
   };

   template<>
   struct explicit_cast_helper<bool, BOOL>
   {
      inline static bool const cast (BOOL const value) throw ()
      {
         return value != FALSE;
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TValueType>
   struct implicit_cast_helper
   {
      inline explicit implicit_cast_helper (TValueType const & value_)
         :  value (value_)
      {
      }

      template<typename TCastToType>
      inline operator TCastToType () const
      {
         return explicit_cast_helper<TCastToType, TValueType>::cast (value);
      }

   private:
      TValueType const & value;
   };
   // -------------------------------------------------------------------------
   
   // -------------------------------------------------------------------------
   template<typename TValueType>
   inline implicit_cast_helper<TValueType> const implicit_cast (
         TValueType const & value
      )
   {
      return implicit_cast_helper<TValueType> (value);
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
