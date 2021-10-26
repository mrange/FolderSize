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
   file_time const to_file_time (FILETIME const ft) noexcept;
   FILETIME const get_current_time () noexcept;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   tstring const get_window_text (HWND const hwnd);
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct handle : boost::noncopyable
   {
      explicit handle (HANDLE const hnd) noexcept;
      handle (handle const &&);
      ~handle () noexcept;
      bool const is_valid () const noexcept;

      HANDLE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct dll : boost::noncopyable
   {
      explicit dll (LPCTSTR const dll_name) noexcept;
      dll (const dll &&);
      ~dll () noexcept;
      bool const is_valid () const noexcept;

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

      bool const is_valid () const noexcept
      {
         return value != nullptr;
      }

      TFunctionPtr const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct thread : boost::noncopyable
   {
      typedef std::function<unsigned int ()> proc;
      thread (
            tstring const & thread_name
         ,  proc const del);
      thread (thread const &&);

      bool const join (unsigned int const ms) const noexcept;
      bool const is_terminated () const noexcept;

   private:
      static void raw_proc (void * ptr) noexcept;

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
      ~find_file () noexcept;

      bool const     is_valid () const noexcept;
      bool const     find_next () noexcept;
      bool const     is_directory () const noexcept;
      big_size const get_size () const noexcept;
      FILETIME const get_creation_time () const noexcept;
      FILETIME const get_last_access_time () const noexcept;
      FILETIME const find_file::get_last_write_time () const noexcept;
      DWORD const    get_reparse_point_tag () const noexcept;
      DWORD const    get_file_attributes () const noexcept;
      LPCTSTR const  get_name () const noexcept;

   private:
      WIN32_FIND_DATA   find_data;
      HANDLE const      find_file_handle;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename ValueType>
   struct thread_safe_scoped_ptr
   {
      explicit thread_safe_scoped_ptr (ValueType * const ptr = nullptr) noexcept
         :  m_ptr (ptr)
      {
      }

      std::auto_ptr<ValueType> reset (ValueType * const ptr = nullptr) noexcept
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

      void set () noexcept;

      handle const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct paint_device_context : boost::noncopyable
   {
      explicit paint_device_context (HWND const hwnd) noexcept;
      paint_device_context (paint_device_context const &&);
      ~paint_device_context () noexcept;

      PAINTSTRUCT const paint_struct   ;
      HWND const        hwnd           ;
      HDC const         hdc            ;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct window_device_context : boost::noncopyable
   {
      explicit window_device_context (HWND const hwnd) noexcept;
      window_device_context (window_device_context const &&);
      ~window_device_context () noexcept;

      HWND const  hwnd;
      HDC const   hdc;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct device_context : boost::noncopyable
   {
      explicit device_context (HDC const dc) noexcept;
      device_context (device_context const &&);
      ~device_context () noexcept;

      HDC const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   template<typename TGdiObject>
   struct gdi_object : boost::noncopyable
   {
      explicit gdi_object (TGdiObject const obj) noexcept
         :  value (obj)
      {
      }
      gdi_object (gdi_object const &&);

      bool const is_valid () const noexcept
      {
         return value != nullptr;
      }

      ~gdi_object () noexcept
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
      select_object (HDC const dc, HGDIOBJ obj) noexcept;
      select_object (select_object const &&);
      ~select_object () noexcept;

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
   bool const is_inside (RECT const & rect, POINT const & point) noexcept;
   boost::optional<RECT> const intersect (RECT const & left, RECT const & right) noexcept;
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
            std::is_pod<TValue>::value,
            "TValue must be pod-type"
            );

         static_assert (
            sizeof (long long) == 8,
            "sizeof (long long) must be 8"
            );

         atomic_impl () noexcept
            :  m_value (0)
         {
         }

         atomic_impl (TValue const value) noexcept
            :  m_value ((long long)value)
         {
         }

         WIN32_INLINE TValue const get () const noexcept
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
            ) noexcept
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
            std::is_pod<TValue>::value,
            "TValue must be pod-type"
            );

         static_assert (
            sizeof (long) == 4,
            "sizeof (long) must be 4"
            );

         atomic_impl () noexcept
            :  m_value (0)
         {
         }

         explicit atomic_impl (TValue const value) noexcept
            :  m_value ((long)value)
         {
         }

         WIN32_INLINE TValue const get () const noexcept
         {
            // No interlocked needed on 32bit as memory bus is 32 bit and m_value is aligned
            // Also VS2005+ puts in a memory barrier implicitly on volatile
            // (even though it's not mandated by the standard)
            return (TValue) m_value;
         }

         WIN32_INLINE bool const compare_and_exchange (
            TValue const new_value,
            TValue const compare_value
            ) noexcept
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
         std::is_pod<TValue>::value,
         "TValue must be pod-type"
         );

      atomic () noexcept
      {
      }

      explicit atomic (TValue const value) noexcept
         :  m_value (value)
      {
      }

      WIN32_INLINE TValue const get () const noexcept
      {
         return m_value.get ();
      }

      WIN32_INLINE bool const compare_and_exchange (
         TValue const new_value,
         TValue const compare_value
         ) noexcept
      {
         return m_value.compare_and_exchange (
            new_value,
            compare_value
            );
      }

      template<typename TFunctor>
      WIN32_INLINE TValue const update (TFunctor const functor) noexcept
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

      WIN32_INLINE TValue const inplace_add (TValue const value) noexcept
      {
         return update ([=] (TValue const v) { return value + v; });
      }

      WIN32_INLINE TValue const inplace_max (TValue const value) noexcept
      {
         return update ([=] (TValue const v) { return max_impl (value, v); });
      }

      WIN32_INLINE TValue const inplace_min (TValue const value) noexcept
      {
         return update ([=] (TValue const v) { return min_impl (value, v); });
      }

   private:

      WIN32_INLINE static TValue max_impl (TValue const left, TValue const right) noexcept
      {
         return left < right ? right : left;
      }

      WIN32_INLINE static TValue min_impl (TValue const left, TValue const right) noexcept
      {
         return left < right ? left : right;
      }

      detail::atomic_impl<TValue, sizeof (TValue)> m_value;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
