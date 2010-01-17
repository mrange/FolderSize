// ----------------------------------------------------------------------------
#pragma once
// ----------------------------------------------------------------------------
#include <tchar.h>
#include <windows.h>
// ----------------------------------------------------------------------------
#include <functional>
#include <memory>
// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
// ----------------------------------------------------------------------------
#include "../folder.hpp"
// ----------------------------------------------------------------------------
namespace painter
{
   // -------------------------------------------------------------------------
   struct painter : boost::noncopyable
   {
      struct impl;

      typedef std::tr1::function<folder::folder const * ()> folder_getter;

      painter (folder_getter const getter);

      void paint (HDC const hdc, int width, int height);

      void set_size (int width, int height);
   private:
      std::auto_ptr<impl> m_impl;
   };
   // -------------------------------------------------------------------------
}
// ----------------------------------------------------------------------------
