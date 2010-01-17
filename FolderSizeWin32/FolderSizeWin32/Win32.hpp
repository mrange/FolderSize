// ----------------------------------------------------------------------------
#pragma once
// ----------------------------------------------------------------------------
#include <windows.h>
// ----------------------------------------------------------------------------
#include <functional>
#include <string>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
namespace win32
{
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   typedef std::basic_string<TCHAR> tstring;
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct handle : boost::noncopyable
   {
      handle (HANDLE hnd) throw ();
      ~handle () throw ();
      bool const is_valid () const throw ();

      HANDLE const value;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct thread : boost::noncopyable
   {
      typedef std::tr1::function<unsigned int ()> proc;
      thread (proc const del);

      proc const        procedure;
      handle const      value;

      bool const join (DWORD const ms) const throw ();
      bool const is_terminated () const throw ();

   private:
      static void raw_proc (void * ptr) throw ();

      bool volatile        terminated;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
   struct find_file : boost::noncopyable
   {
      find_file (tstring const & path);

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
}
// ----------------------------------------------------------------------------
