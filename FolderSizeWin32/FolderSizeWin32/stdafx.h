// ----------------------------------------------------------------------------
#pragma once
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN

#include "targetver.h"
#include <windows.h>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <tchar.h>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>
#include <atlstr.h>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32'  name='Microsoft.Windows.Common-Controls'  version='6.0.0.0'  processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' 
    name='Microsoft.Windows.Common-Controls'  version='6.0.0.0' 
    processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' 
    language='*'\"")
#endif
// ----------------------------------------------------------------------------
