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
namespace linear
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct no_initialize
   {
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
         auto values_ptr = values;
         auto default_value = value_type ();

         for(auto iter = 0; iter < no_of_values; ++iter, ++values_ptr)
         {
            *values_ptr = default_value;
         }
      }

      vector (no_initialize const no_init)
      {
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
         auto values_ptr = values;
         auto default_value = value_type ();

         for(auto iter = 0; iter < no_of_values; ++iter, ++values_ptr)
         {
            *values_ptr = default_value;
         }
      }

      matrix (no_initialize const no_init)
      {
      }

      value_type  values[no_of_values];
   };
   // -------------------------------------------------------------------------
   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator+ (
      matrix<value_type_, rows_, columns_> const & left,
      matrix<value_type_, rows_, columns_> const & right)
   {
      left::type matrix = left;

      for(auto iter = 0; iter < left::no_of_values; ++iter)
      {
         left += right.values[iter];
      }
   }

   template<typename value_type_, std::size_t rows_, std::size_t columns_>
   matrix<value_type_, rows_, columns_> const operator- (
         matrix<value_type_, rows_, columns_> const & left
      ,  matrix<value_type_, rows_, columns_> const & right)
   {
      left::type matrix = left;

      for(auto iter = 0; iter < left::no_of_values; ++iter)
      {
         left -= right.values[iter];
      }
   }

   template<typename value_type_, std::size_t left_rows_, std::size_t shared_dimension_, std::size_t right_columns_>
   matrix<value_type_, left_rows_, right_columns_> const operator* (
         matrix<value_type_, left_rows_, shared_dimension_> const & left
      ,  matrix<value_type_, shared_dimension_, right_columns_> const & right)
   {
      no_initialize no_init;
      matrix<value_type_, left_rows_, right_columns_> result (no_init);

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
      no_initialize no_init;
      vector<value_type_, rows_> result (no_init);

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
   matrix<value_type_, dimension_, dimension_> const translate(
      vector<value_type_, dimension_> const & offset)
   {
      matrix<value_type_, dimension_, dimension_> result;
      auto result_values_ptr = result.values + (dimension_ - 1);

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_)
      {
         *result_values_ptr = offset.values[iter];
      }

      return result;
   }

   template<typename value_type_, std::size_t dimension_>
   matrix<value_type_, dimension_, dimension_> const scale(
      vector<value_type_, dimension_> const & scaling)
   {
      matrix<value_type_, dimension_, dimension_> result;
      auto result_values_ptr = result.values;

      for (auto iter = 0; iter < dimension_; ++iter, result_values_ptr += dimension_ + 1)
      {
         *result_values_ptr = scaling.values[iter];
      }

      return result;
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
