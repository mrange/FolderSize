// ----------------------------------------------------------------------------
#include "stdafx.h"
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
namespace main_window
{
   int                  application_main_loop (
                           HINSTANCE hInstance,
                           HINSTANCE hPrevInstance,
                           LPTSTR    lpCmdLine,
                           int       nCmdShow);
}
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
int APIENTRY _tWinMain (
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPTSTR    lpCmdLine,
   int       nCmdShow)
{
   main_window::application_main_loop (
      hInstance,
      hPrevInstance,
      lpCmdLine,
      nCmdShow);
}
// ----------------------------------------------------------------------------
