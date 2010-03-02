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
#include <boost/optional.hpp>
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
   extern bool const windows7_or_later                                           ;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TValueType>
   TValueType const get_windows7_dependent_value (
         TValueType const & windows7_value
      ,  TValueType const & legacy_value
      )
   {
      if (windows7_or_later)
      {
         return windows7_value;
      }
      else
      {
         return legacy_value;
      }
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   void trace_string (tstring const & value);
   void trace_string (LPCTSTR const value);

   void debug_string (tstring const & value);
   void debug_string (LPCTSTR const value);

#define WIN32_PRELUDE _T (__FUNCTION__) _T (" : ")
#ifdef _DEBUG
#  define WIN32_DEBUG_STRING(expr) (::win32::debug_string (expr))
#else
#  define WIN32_DEBUG_STRING(expr)
#endif
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   typedef unsigned __int64 file_time  ;
   file_time const ticks_per_day = 24 * 60 * 60 * (1000000000ui64 / 100ui64);
   // -------------------------------------------------------------------------
   file_time const to_file_time (FILETIME const ft) throw ();
   FILETIME const get_current_time () throw ();
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   tstring const get_window_text (HWND const hwnd);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct handle : boost::noncopyable
   {
      explicit handle (HANDLE const hnd) throw ();
      handle (handle const &&);
      ~handle () throw ();
      bool const is_valid () const throw ();

      HANDLE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct dll : boost::noncopyable
   {
      explicit dll (LPCTSTR const dll_name) throw ();
      dll (const dll &&);
      ~dll () throw ();
      bool const is_valid () const throw ();

      HMODULE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TFunctionPtr>
   struct function_pointer : boost::noncopyable
   {
      function_pointer (HMODULE module, LPCSTR const function_name)
         :  value (reinterpret_cast<TFunctionPtr> (GetProcAddress (module, function_name)))
      {
      }
      function_pointer (function_pointer const &&);

      bool const is_valid () const throw ()
      {
         return value != NULL;
      }

      TFunctionPtr const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct thread : boost::noncopyable
   {
      typedef std::tr1::function<unsigned int ()> proc;
      thread (
            tstring const & thread_name
         ,  proc const del);
      thread (thread const &&);

      bool const join (unsigned int const ms) const throw ();
      bool const is_terminated () const throw ();

   private:
      static void raw_proc (void * ptr) throw ();

      tstring const        thread_name;
      bool volatile        terminated;
   public:
      proc const        procedure;
      handle const      value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct find_file : boost::noncopyable
   {
      explicit find_file (tstring const & path);
      find_file (find_file const &&);
      ~find_file () throw ();

      bool const     is_valid () const throw ();
      bool const     find_next () throw ();
      bool const     is_directory () const throw ();
      big_size const get_size () const throw ();
      FILETIME const get_creation_time () const throw ();
      FILETIME const get_last_access_time () const throw ();
      FILETIME const find_file::get_last_write_time () const throw ();
      DWORD const    get_reparse_point_tag () const throw (); 
      DWORD const    get_file_attributes () const throw ();
      LPCTSTR const  get_name () const throw ();

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
               ,  pointer
               ))
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
   namespace event_type
   {
      enum type
      {
         auto_reset     ,
         manual_reset   ,
      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct event : boost::noncopyable
   {
      explicit event (event_type::type const event_type);
      event (event const &&);

      void set () throw ();

      handle const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct paint_device_context : boost::noncopyable
   {
      explicit paint_device_context (HWND const hwnd) throw ();
      paint_device_context (paint_device_context const &&);
      ~paint_device_context () throw ();

      PAINTSTRUCT const paint_struct   ;
      HWND const        hwnd           ;
      HDC const         hdc            ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct window_device_context : boost::noncopyable
   {
      explicit window_device_context (HWND const hwnd) throw ();
      window_device_context (window_device_context const &&);
      ~window_device_context () throw ();

      HWND const  hwnd;
      HDC const   hdc;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct device_context : boost::noncopyable
   {
      explicit device_context (HDC const dc) throw ();
      device_context (device_context const &&);
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
      gdi_object (gdi_object const &&);

      bool const is_valid () const throw ()
      {
         return value != NULL;
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
      select_object (select_object const &&);
      ~select_object () throw ();

      HDC const      dc                         ;
      HGDIOBJ const  previously_selected_object ;
   };
   // -------------------------------------------------------------------------
   void gradient_fill (
         HDC const hdc
      ,  RECT const & rect
      ,  COLORREF const top_color
      ,  COLORREF const bottom_color
      );
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   gdi_object<HFONT> const create_font (LPCTSTR const font_family, int const height);
   // -------------------------------------------------------------------------
   namespace standard_font
   {
      enum type
      {
         caption  ,
         menu     ,
         status   ,
         message  ,
      };
   }
   // -------------------------------------------------------------------------
   gdi_object<HFONT> const create_standard_font (
         standard_font::type const font_type
      ,  int const height
      );
   gdi_object<HFONT> const create_standard_font (
         standard_font::type const font_type
      );
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   tstring const load_string_resource (int const resource_id);
   tstring const load_string_resource (int const resource_id, LPCTSTR const default_value);
   gdi_object<HBITMAP> const load_bitmap_resource (int const resource_id);
   SIZE const get_bitmap_size (HDC const hdc, HBITMAP const bitmap);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   RECT const  zero_rect ();
   bool const is_inside (RECT const & rect, POINT const & point) throw ();
   boost::optional<RECT> const intersect (RECT const & left, RECT const & right) throw ();
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   POINT const get_client_mouse_coordinate (HWND const hwnd, LPARAM const lParam);
   POINT const get_mouse_coordinate (LPARAM const lParam);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
