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
#include <type_traits>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
// ----------------------------------------------------------------------------
#include "utility.hpp"
// ----------------------------------------------------------------------------
#define WIN32_INLINE inline
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
         return value != nullptr;
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
      explicit thread_safe_scoped_ptr (ValueType * const ptr = nullptr) throw ()
         :  m_ptr (ptr)
      {
      }

      std::auto_ptr<ValueType> reset (ValueType * const ptr = nullptr) throw ()
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
         enum_count     ,
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
         return value != nullptr;
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
         caption     ,
         menu        ,
         status      ,
         message     ,
         enum_count  ,
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
   namespace detail
   {
      template<typename TValue, int size>
      struct atomic_impl;


      template<typename TValue>
      struct atomic_impl<TValue, 8>
      {
         static_assert (
            std::tr1::is_pod<TValue>::value,
            "TValue must be pod-type"
            );

         static_assert (
            sizeof (long long) == 8,
            "sizeof (long long) must be 8"
            );

         atomic_impl () throw ()
            :  m_value (0)
         {
         }

         atomic_impl (TValue const value) throw ()
            :  m_value ((long long)value)
         {
         }

         WIN32_INLINE TValue const get () const throw ()
         {
            return (TValue) (
               _InterlockedCompareExchange64 (
                     const_cast<long long volatile*>(&m_value)
                  ,  0
                  ,  0
                  ));
         }

         WIN32_INLINE bool const compare_and_exchange (
            TValue const new_value,
            TValue const compare_value
            ) throw ()
         {
            auto new_value_      = (long long)new_value;
            auto compare_value_  = (long long)compare_value;
            return 
               _InterlockedCompareExchange64 (
                     &m_value
                  ,  new_value_
                  ,  compare_value_
                  )
               == compare_value_;
         }

      private:
         __declspec (align (8))
         volatile long long m_value;
      };


      template<typename TValue>
      struct atomic_impl<TValue, 4>
      {
         static_assert (
            std::tr1::is_pod<TValue>::value,
            "TValue must be pod-type"
            );

         static_assert (
            sizeof (long) == 4,
            "sizeof (long) must be 4"
            );

         atomic_impl () throw ()
            :  m_value (0)
         {
         }

         explicit atomic_impl (TValue const value) throw ()
            :  m_value ((long)value)
         {
         }

         WIN32_INLINE TValue const get () const throw ()
         {
            // No interlocked needed on 32bit as memory bus is 32 bit and m_value is aligned 
            // Also VS2005+ puts in a memory barrier implicitly on volatile 
            // (even though it's not mandated by the standard)
            return (TValue) m_value;
         }

         WIN32_INLINE bool const compare_and_exchange (
            TValue const new_value,
            TValue const compare_value
            ) throw ()
         {
            auto new_value_      = (long)new_value;
            auto compare_value_  = (long)compare_value;
            return 
               _InterlockedCompareExchange (
                     &m_value
                  ,  new_value_
                  ,  compare_value_
                  )
               == compare_value_;
         }

      private:
         __declspec (align (4))
         volatile long m_value;
      };
   }
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TValue>
   struct atomic
   {
      static_assert (
         std::tr1::is_pod<TValue>::value,
         "TValue must be pod-type"
         );

      atomic () throw ()
      {
      }

      explicit atomic (TValue const value) throw ()
         :  m_value (value)
      {
      }

      WIN32_INLINE TValue const get () const throw ()
      {
         return m_value.get ();
      }

      WIN32_INLINE bool const compare_and_exchange (
         TValue const new_value,
         TValue const compare_value
         ) throw ()
      {
         return m_value.compare_and_exchange (
            new_value,
            compare_value
            );
      }

      template<typename TFunctor>
      WIN32_INLINE TValue const update (TFunctor const functor) throw ()
      {
         auto current_value   = TValue ();
         auto new_value       = TValue ();
         do
         {
            current_value = m_value.get ();
            new_value = functor (current_value);
         }
         while (!m_value.compare_and_exchange (new_value, current_value));

         return new_value;
      }

      WIN32_INLINE TValue const inplace_add (TValue const value) throw ()
      {
         return update ([=] (TValue const v) { return value + v; });
      }

      WIN32_INLINE TValue const inplace_max (TValue const value) throw ()
      {
         return update ([=] (TValue const v) { return max_impl (value, v); });
      }

      WIN32_INLINE TValue const inplace_min (TValue const value) throw ()
      {
         return update ([=] (TValue const v) { return min_impl (value, v); });
      }

   private:

      WIN32_INLINE static TValue max_impl (TValue const left, TValue const right) throw ()
      {
         return left < right ? right : left;
      }

      WIN32_INLINE static TValue min_impl (TValue const left, TValue const right) throw ()
      {
         return left < right ? left : right;
      }

      detail::atomic_impl<TValue, sizeof (TValue)> m_value;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
