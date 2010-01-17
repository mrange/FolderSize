// ----------------------------------------------------------------------------
#pragma once
// ----------------------------------------------------------------------------
#include <tchar.h>
// ----------------------------------------------------------------------------
#include <string>
#include <vector>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
namespace folder
{
   // -------------------------------------------------------------------------
   typedef std::basic_string<TCHAR> tstring;
   // -------------------------------------------------------------------------
   struct folder : boost::noncopyable
   {
      typedef std::vector<folder const * const> list;

      struct initializer
      {
         __int64 const     size;
         __int64 const     file_count;
         tstring const &   name;
         std::size_t const sub_folders_size;

         initializer (
            __int64 const     sz,
            __int64 const     fc,
            tstring const &   n,
            std::size_t const sfz
            );
      };

      folder ();

      folder (
         initializer const & init);

      __int64 const        size;
      __int64 const        file_count;
      tstring const        name;
      list const           sub_folders;

      static folder const  empty;
   };
   // -------------------------------------------------------------------------

   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
