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
#include <cstddef>
#include <type_traits>
// ----------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
// ----------------------------------------------------------------------------
#define LINEAR_INLINE __forceinline
// ----------------------------------------------------------------------------
namespace linear
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct no_initialize
   {
      static no_initialize value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_>
   struct vector
   {
      typedef value_type_ value_type;
      enum
      {
         rows           = rows_  ,
         columns        = 1      ,
         no_of_values   = rows   ,
      };

      BOOST_STATIC_ASSERT (rows > 0);
      BOOST_STATIC_ASSERT (std::tr1::is_pod<value_type>::value);

      typedef vector<value_type, rows> type;

      vector ()
      {
         memset (values, 0, sizeof (values));
      }

      LINEAR_INLINE explicit vector (no_initialize const no_init)
      {
      }

      LINEAR_INLINE value_type const operator() (std::size_t const row) const throw ()
      {
         BOOST_ASSERT (row < no_of_values);
         return values[row];
      }

      LINEAR_INLINE value_type const x () const throw ()
      {
         BOOST_STATIC_ASSERT (0 < no_of_values);
         return values[0];
      }

      LINEAR_INLINE value_type const y () const throw ()
      {
         BOOST_STATIC_ASSERT (1 < no_of_values);
         return values[1];
      }

      LINEAR_INLINE value_type const z () const throw ()
      {
         BOOST_STATIC_ASSERT (2 < no_of_values);
         return values[2];
      }

      LINEAR_INLINE void x (value_type const x_) throw ()
      {
         BOOST_STATIC_ASSERT (0 < no_of_values);
         values[0] = x_;
      }

      LINEAR_INLINE void y (value_type const y_) throw ()
      {
         BOOST_STATIC_ASSERT (1 < no_of_values);
         values[1] = y_;
      }

      LINEAR_INLINE void z (value_type const z_) throw ()
      {
         BOOST_STATIC_ASSERT (2 < no_of_values);
         values[2] = z_;
      }


      LINEAR_INLINE vector const operator- () const throw ()
      {
         vector result (no_initialize::value);
         for (auto iter = 0; iter < no_of_values; ++iter)
         {
            result.values[iter] = -values[iter];
         }
         return result;
      }

      value_type  values[no_of_values];
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_ + 1> const extend_vector (vector<value_type_, rows_> const & value)
   {
      vector<value_type_, rows_ + 1> v (no_initialize::value);

      memcpy (v.values, value.values, sizeof (value.values));
      v.values [vector<value_type_, rows_>::no_of_values - 1] = 1;

      return v;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_ - 1> const shrink_vector (vector<value_type_, rows_> const & value)
   {
      vector<value_type_, rows_ - 1> v (no_initialize::value);

      BOOST_ASSERT ((value.values [vector<value_type_, rows_>::no_of_values - 1] == 1));

      memcpy (v.values, value.values, sizeof (value.values) - sizeof (value_type_));

      return v;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const invert_vector (
         vector<value_type_, rows_> const & value
      )
   {
      vector<value_type_, rows_> v (no_initialize::value);

      for (auto iter = 0; iter < rows_; ++iter)
      {
         v.values[iter] = 1 / value.values[iter];
      }

      return v;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const scale_vector (
         vector<value_type_, rows_> const & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      vector<value_type_, rows_> v (no_initialize::value);

      for (auto iter = 0; iter < rows_; ++iter)
      {
         v.values[iter] = left.values[iter] * right.values[iter];
      }

      return v;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const scale_vector (
         value_type_ const left
      ,  vector<value_type_, rows_> const & right
      )
   {
      vector<value_type_, rows_> v (no_initialize::value);

      for (auto iter = 0; iter < rows_; ++iter)
      {
         v.values[iter] = left * right.values[iter];
      }

      return v;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const operator+= (
         vector<value_type_, rows_> & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      for (auto iter = 0; iter < vector<value_type_, rows_>::no_of_values; ++iter)
      {
         left.values[iter] += right.values[iter];
      }

      return left;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const operator+ (
         vector<value_type_, rows_> const & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      auto v = left;
      v += right;
      return v;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const operator-= (
         vector<value_type_, rows_> & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      for (auto iter = 0; iter < vector<value_type_, rows_>::no_of_values; ++iter)
      {
         left.values[iter] -= right.values[iter];
      }

      return left;
   }

   template<typename value_type_, std::size_t rows_>
   vector<value_type_, rows_> const operator- (
         vector<value_type_, rows_> const & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      auto v = left;
      v -= right;
      return v;
   }

   // -------------------------------------------------------------------------


   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   struct matrix
   {
      typedef value_type_ value_type;
      enum
      {
         rows           = rows_           ,
         columns        = columns_        ,
         no_of_values   = rows * columns  ,
      };

      BOOST_STATIC_ASSERT (columns  > 0);
      BOOST_STATIC_ASSERT (rows     > 0);
      BOOST_STATIC_ASSERT (std::tr1::is_pod<value_type>::value);

      typedef matrix<value_type, rows, columns> type;

      matrix ()
      {
         memset (values, 0, sizeof (values));
      }

      LINEAR_INLINE explicit matrix (no_initialize const no_init)
      {
      }

      LINEAR_INLINE value_type const operator() (std::size_t const row, std::size_t const column) const throw ()
      {
         BOOST_ASSERT (column + columns * row < no_of_values);
         return values[column + columns * row];
      }

      value_type  values[no_of_values];
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, columns_, rows_> const transpose_matrix (
         matrix<value_type_, rows_, columns_> const & value
      ) throw ()
   {
      matrix<value_type_, columns_, rows_> result (no_initialize::value);

      for (auto iter = 0; iter < no_of_values; ++iter)
      {
         result.values[iter] = values[iter / 3 + (iter % 3) * columns];
      }

      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   bool const operator== (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right
      )
   {
      auto result = true;
      for (auto iter = 0; iter < matrix<value_type_, rows_, columns_>::no_of_values; ++iter)
      {
         result &= left.values[iter] == right.values[iter];
      }
      return result;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> & operator+= (
         matrix<value_type_, rows_, columns_> & left
      ,  matrix<value_type_, rows_, columns_> const & right
      )
   {
      for (auto iter = 0; iter < matrix<value_type_, rows_, columns_>::no_of_values; ++iter)
      {
         left.values[iter] += right.values[iter];
      }

      return left;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> operator+ (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right
      )
   {
      auto m = left;
      m += right;
      return m;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> & operator-= (
         matrix<value_type_, rows_, columns_> & left
      ,  matrix<value_type_, rows_, columns_> const & right
      )
   {
      for (auto iter = 0; iter < matrix<value_type_, rows_, columns_>::no_of_values; ++iter)
      {
         left.values[iter] -= right.values[iter];
      }

      return left;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> operator- (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right
      )
   {
      auto m = left;
      m -= right;
      return m;
   }

   template<typename value_type_, std::size_t left_rows_, std::size_t shared_dimension_, std::size_t right_columns_>
   matrix<value_type_, left_rows_, right_columns_> const operator* (
         matrix<value_type_, left_rows_, shared_dimension_> const & left
      ,  matrix<value_type_, shared_dimension_, right_columns_> const & right
      )
   {
      matrix<value_type_, left_rows_, right_columns_> result (no_initialize::value);

      auto result_values_ptr = result.values;
      auto right_step = shared_dimension_;
      auto default_value = value_type_ ();


      for (auto row = 0; row < left_rows_; ++row)
      {
         auto left_values_ptr = left.values + row * shared_dimension_;

         for (auto column = 0; column < right_columns_; ++column)
         {
            auto col_values_ptr = left_values_ptr;
            auto row_values_ptr = right.values + column;
            auto sub_result = default_value;

            for (auto iter = 0; iter < shared_dimension_; ++iter)
            {
               sub_result += (*col_values_ptr) * (*row_values_ptr);
               ++col_values_ptr;
               row_values_ptr += right_columns_;
            }

            *result_values_ptr++ = sub_result;
         }
      }

      return result;
   }
   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   vector<value_type_, rows_> const operator* (
         matrix<value_type_, rows_, columns_> const & left
      ,  vector<value_type_, columns_> const & right
      )
   {
      vector<value_type_, rows_> result (no_initialize::value);

      auto result_values_ptr = result.values;
      auto default_value = value_type_ ();
      auto left_values_ptr = left.values;

      for (auto row = 0; row < rows_; ++row)
      {
         auto right_values_ptr = right.values;
         auto sub_result = default_value;

         for (auto iter = 0; iter < columns_; ++iter)
         {
            sub_result += (*left_values_ptr) * (*right_values_ptr);
            ++left_values_ptr;
            ++right_values_ptr;
         }

         *result_values_ptr++ = sub_result;
      }

      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_>
   bool const operator== (
         vector<value_type_, rows_> const & left
      ,  vector<value_type_, rows_> const & right
      )
   {
      auto result = true;
      for (auto iter = 0; iter < vector<value_type_, rows_>::no_of_values; ++iter)
      {
         result &= left.values[iter] == right.values[iter];
      }
      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_, dimension_> const zero_matrix()
   {
      return matrix<value_type_, dimension_, dimension_> ();
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_, dimension_> const identity_matrix()
   {
      matrix<value_type_, dimension_, dimension_> result;

      auto result_values_ptr = result.values;
      auto default_value = 1;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 1)
      {
         *result_values_ptr = default_value;
      }

      return result;
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_ + 1, dimension_ + 1> const translating_matrix(
      vector<value_type_, dimension_> const & offset
      )
   {
      auto result = identity_matrix<value_type_, dimension_ + 1> ();
      auto result_values_ptr = result.values + dimension_;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 1)
      {
         *result_values_ptr = offset.values[iter];
      }

      return result;
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_ + 1, dimension_ + 1> const scaling_matrix(
      vector<value_type_, dimension_> const & scaling
      )
   {
      auto result = identity_matrix<value_type_, dimension_ + 1> ();
      auto result_values_ptr = result.values;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 2)
      {
         *result_values_ptr = scaling.values[iter];
      }

      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
