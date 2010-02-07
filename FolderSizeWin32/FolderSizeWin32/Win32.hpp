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
#include <windows.h>
// ----------------------------------------------------------------------------
#include <functional>
#include <memory>
#include <string>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "utility.hpp"
// ----------------------------------------------------------------------------
namespace win32
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   typedef std::basic_string<TCHAR> tstring;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void output_debug_string (tstring const & value);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct handle : boost::noncopyable
   {
      explicit handle (HANDLE const hnd) throw ();
      ~handle () throw ();
      bool const is_valid () const throw ();

      HANDLE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct thread : boost::noncopyable
   {
      typedef std::tr1::function<unsigned int ()> proc;
      thread (
         tstring const & thread_name,
         proc const del);

      proc const        procedure;
      handle const      value;

      bool const join (unsigned int const ms) const throw ();
      bool const is_terminated () const throw ();

   private:
      static void raw_proc (void * ptr) throw ();

      tstring const        thread_name;
      bool volatile        terminated;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct find_file : boost::noncopyable
   {
      explicit find_file (tstring const & path);

      bool const is_valid () const throw ();
      bool const find_next () throw ();
      bool const is_directory () const throw ();
      __int64 const get_size () const throw ();
      LPCTSTR const get_name () const throw ();
      ~find_file () throw ();

   private:
      WIN32_FIND_DATA   find_data;
      HANDLE const      find_file_handle;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename ValueType>
   struct thread_safe_scoped_ptr
   {
      explicit thread_safe_scoped_ptr (ValueType * const ptr = NULL) throw ()
         :  m_ptr (ptr)
      {
      }

      std::auto_ptr<ValueType> reset (ValueType * const ptr = NULL) throw ()
      {
         auto pointer = m_ptr;

         while (
            pointer != InterlockedCompareExchangePointer (
                  &m_ptr
               ,  ptr
               ,  pointer))
         {
            pointer = m_ptr;
         }

         return std::auto_ptr<ValueType> (reinterpret_cast<ValueType*> (pointer));
      }

   private:
      void * volatile m_ptr;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct event : boost::noncopyable
   {
      explicit event (bool const auto_reset = false);

      void set () throw ();

      handle const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct window_device_context : boost::noncopyable
   {
      explicit window_device_context (HWND const hwnd) throw ();
      ~window_device_context () throw ();

      HWND const  hwnd;
      HDC const   hdc;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct device_context : boost::noncopyable
   {
      explicit device_context (HDC const dc) throw ();
      ~device_context () throw ();

      HDC const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TGdiObject>
   struct gdi_object : boost::noncopyable
   {
      explicit gdi_object (TGdiObject const obj) throw ()
         :  value (obj)
      {
      }

      ~gdi_object () throw ()
      {
         if (value)
         {
            auto deleted_result = DeleteObject (value);
            UNUSED_VARIABLE (deleted_result);
         }
      }

      TGdiObject const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct select_object : boost::noncopyable
   {
      select_object (HDC const dc, HGDIOBJ obj) throw ();
      ~select_object () throw ();

      HDC const      dc                         ;
      HGDIOBJ const  previously_selected_object ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct set_world_transform : boost::noncopyable 
   {
      set_world_transform (HDC const dc, XFORM const * const transform) throw ();
      ~set_world_transform () throw ();

      HDC const      dc                         ;
      XFORM const    old_transform              ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
