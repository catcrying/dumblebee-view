#ifndef __SIMPLE_SOCKET_H__
#define __SIMPLE_SOCKET_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <XUtil/export.h>

XUTIL_API void WINAPI XSimpleSocketInitialize();
XUTIL_API void WINAPI XSimpleSocketTerminate();

#pragma comment(lib, "ws2_32.lib")

#endif //__SIMPLE_SOCKET_H__