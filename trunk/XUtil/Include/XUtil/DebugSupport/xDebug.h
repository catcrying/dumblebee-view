#ifndef __X_DEBUG_H__
#define __X_DEBUG_H__

#include <XUtil/export.h>

#include <wtypes.h>

XUTIL_API BOOL XAssertFailedLineRelease( LPCSTR lpszFileName, int nLine );
XUTIL_API BOOL XAssertFailedLine( LPCSTR lpszFileName, int nLine );

XUTIL_API void XTrace( LPCSTR lpszFormat, ... );
XUTIL_API void XTraceV( LPCSTR lpszFormat, va_list argList );
XUTIL_API void XTrace( LPCWSTR lpszFormat, ... );
XUTIL_API void XTraceV( LPCWSTR lpszFormat, va_list argList );

#if defined(_DEBUG)
#	if defined(_WIN32_WCE)
#		define XASSERT(f) do { if (!(f)) XAssertFailedLineRelease(__FILE__, __LINE__); } while (0)\
	
#		define XVERIFY(f) XASSERT(f)
#		define XTRACE
#	else
#		define XASSERT(f)   do { if (!(f)&&XAssertFailedLine(__FILE__, __LINE__)) DebugBreak(); } while (0)\

#		define XVERIFY(f)   XASSERT(f)
#		define XTRACE       XTrace
#	endif
#else
	#define XASSERT(f) ((void)(f))
	#define XVERIFY(f) XASSERT(f)
	#define XTRACE
#endif

#endif //__X_DEBUG_H__