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
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/scoped_array.hpp>
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#include <atlbase.h>
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
#ifdef UNICODE
#  if defined _M_IX86
#     pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#  elif defined _M_IA64
#     pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#  elif defined _M_X64
#     pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#  else
#     pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#  endif
#endif
// ----------------------------------------------------------------------------
