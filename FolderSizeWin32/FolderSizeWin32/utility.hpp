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
#define UTILITY_INLINE __forceinline
// ----------------------------------------------------------------------------
#define UNUSED_VARIABLE(expr) expr
#define IMPLICIT_CAST(expr) (utility::implicit_cast (expr))
#define EXPLICIT_CAST(type,expr) (utility::explicit_cast<type> (expr))
#define IS_ON utility::is_on
#define IS_OFF utility::is_off
#define ANY_IS_ON utility::any_is_on
#define ANY_IS_OFF utility::any_is_off
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
      UTILITY_INLINE static int const cast (double const &value) noexcept
      {
         return static_cast<int> (value + 0.5);
      }
   };

   template<>
   struct explicit_cast_helper<LONG, double>
   {
      UTILITY_INLINE static LONG const cast (double const &value) noexcept
      {
         return static_cast<LONG> (value + 0.5);
      }
   };

   template<>
   struct explicit_cast_helper<FLOAT, double>
   {
      UTILITY_INLINE static FLOAT const cast (double const &value) noexcept
      {
         return static_cast<FLOAT> (value);
      }
   };

   template<>
   struct explicit_cast_helper<bool, BOOL>
   {
      UTILITY_INLINE static bool const cast (BOOL const value) noexcept
      {
         return value != FALSE;
      }
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TValueType>
   struct implicit_cast_helper
   {
      UTILITY_INLINE explicit implicit_cast_helper (TValueType const & value_)
         :  value (value_)
      {
      }

      template<typename TCastToType>
      UTILITY_INLINE operator TCastToType () const
      {
         return explicit_cast_helper<TCastToType, TValueType>::cast (value);
      }

   private:
      TValueType const & value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TToValueType, typename TFromValueType>
   UTILITY_INLINE TToValueType const explicit_cast (
         TFromValueType const & value
      )
   {
      return explicit_cast_helper<TToValueType, TFromValueType>::cast (value);
   }
   // -------------------------------------------------------------------------
   template<typename TValueType>
   UTILITY_INLINE implicit_cast_helper<TValueType> const implicit_cast (
         TValueType const & value
      )
   {
      return implicit_cast_helper<TValueType> (value);
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TLeft, typename TRight>
   bool const is_on (TLeft const & bits, TRight const & comparand)
   {
      return (bits & static_cast<TLeft> (comparand)) == static_cast<TLeft> (comparand);
   }

   template<typename TLeft, typename TRight>
   bool const is_off (TLeft const & bits, TRight const & comparand)
   {
      return (bits & static_cast<TLeft> (comparand)) == 0;
   }

   template<typename TLeft, typename TRight>
   bool const any_is_on (TLeft const & bits, TRight const & comparand)
   {
      return (bits & static_cast<TLeft> (comparand)) != 0;
   }

   template<typename TLeft, typename TRight>
   bool const any_is_off (TLeft const & bits, TRight const & comparand)
   {
      return (bits & static_cast<TLeft> (comparand)) != static_cast<TLeft> (comparand);
   }
   // -------------------------------------------------------------------------
   template<typename T>
   int const size_of_array (T const & a)
   {
      return sizeof (a) / sizeof (a[0]);
   }
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
