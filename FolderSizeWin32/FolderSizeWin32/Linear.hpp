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
// ----------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
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
      typedef vector<value_type, rows> type;

      vector ()
      {
         auto default_value = value_type ();

         for(auto iter = 0; iter < no_of_values; ++iter)
         {
            values[iter] = default_value;
         }
      }

      vector (no_initialize const no_init)
      {
      }

      value_type const operator() (std::size_t const row) const throw ()
      {
         BOOST_ASSERT (row < no_of_values);
         return values[row];
      }

      value_type const x () const throw ()
      {
         BOOST_STATIC_ASSERT (0 < no_of_values);
         return values[0];
      }

      value_type const y () const throw ()
      {
         BOOST_STATIC_ASSERT (1 < no_of_values);
         return values[1];
      }

      value_type const z () const throw ()
      {
         BOOST_STATIC_ASSERT (2 < no_of_values);
         return values[2];
      }

      void x (value_type const x_) throw ()
      {
         BOOST_STATIC_ASSERT (0 < no_of_values);
         values[0] = x_;
      }

      void y (value_type const y_) throw ()
      {
         BOOST_STATIC_ASSERT (1 < no_of_values);
         values[1] = y_;
      }

      void z (value_type const z_) throw ()
      {
         BOOST_STATIC_ASSERT (2 < no_of_values);
         values[2] = z_;
      }

      value_type  values[no_of_values];
   };
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
      typedef matrix<value_type, rows, columns> type;

      matrix ()
      {
         auto default_value = value_type ();

         for(auto iter = 0; iter < no_of_values; ++iter)
         {
            values[iter] = default_value;
         }
      }

      matrix (no_initialize const no_init)
      {
      }

      value_type const operator() (std::size_t const row, std::size_t const column) const throw ()
      {
         BOOST_ASSERT (column + columns * row < no_of_values);
         return values[column + columns * row];
      }

      matrix const transpose () const throw ()
      {
         matrix result (no_initialize::value);

         for(auto iter = 0; iter < no_of_values; ++iter)
         {
            result.values[iter] = values[iter / 3 + (iter % 3) * columns];
         }

         return result;
      }

      value_type  values[no_of_values];
   };
   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   bool const operator== (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      auto result = true;
      for(auto iter = 0; iter < matrix<value_type_, rows_, columns_>::no_of_values; ++iter)
      {
         result &= left.values[iter] == right.values[iter];
      }
      return result;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator+= (
         matrix<value_type_, rows_, columns_> & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      for(auto iter = 0; iter < left::no_of_values; ++iter)
      {
         left.values[iter] += right.values[iter];
      }
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator+ (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      left::type matrix = left;
      matrix += right;
      return matrix;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator+ (
         matrix<value_type_, rows_, columns_> && left
      ,  matrix<value_type_, rows_, columns_> const && right)
   {
      left += right;
      return left;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator-= (
         matrix<value_type_, rows_, columns_> & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      for(auto iter = 0; iter < left::no_of_values; ++iter)
      {
         left.values[iter] -= right.values[iter];
      }
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator- (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      left::type matrix = left;
      matrix -= right;
      return matrix;
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator- (
         matrix<value_type_, rows_, columns_> && left
      ,  matrix<value_type_, rows_, columns_> const && right)
   {
      left -= right;
      return left;
   }

   template<typename value_type_, std::size_t left_rows_, std::size_t shared_dimension_, std::size_t right_columns_>
   matrix<value_type_, left_rows_, right_columns_> const operator* (
         matrix<value_type_, left_rows_, shared_dimension_> const & left
      ,  matrix<value_type_, shared_dimension_, right_columns_> const & right)
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
      ,  vector<value_type_, columns_> const & right)
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
      ,  vector<value_type_, rows_> const & right)
   {
      auto result = true;
      for(auto iter = 0; iter < vector<value_type_, rows_>::no_of_values; ++iter)
      {
         result &= left.values[iter] == right.values[iter];
      }
      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_, dimension_> const zero()
   {
      return matrix<value_type_, dimension_, dimension_> ();
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_, dimension_> const identity()
   {
      matrix<value_type_, dimension_, dimension_> result;

      auto result_values_ptr = result.values;
      auto default_value = value_type_ ();

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 1)
      {
         *result_values_ptr = default_value;
      }

      return result;
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_ + 1, dimension_ + 1> const translate(
      vector<value_type_, dimension_> const & offset)
   {
      matrix<value_type_, dimension_ + 1, dimension_ + 1> result;
      auto result_values_ptr = result.values + dimension_;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 1)
      {
         *result_values_ptr = offset.values[iter];
      }

      *result_values_ptr = 1;

      return result;
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_ + 1, dimension_ + 1> const scale(
      vector<value_type_, dimension_> const & scaling)
   {
      matrix<value_type_, dimension_ + 1, dimension_ + 1> result;
      auto result_values_ptr = result.values;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 2)
      {
         *result_values_ptr = scaling.values[iter];
      }

      *result_values_ptr = 1;

      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
