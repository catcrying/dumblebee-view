#ifndef __X_UTILS_H__
#define __X_UTILS_H__

#if _MSC_VER > 1000
#pragma once
#endif

#pragma warning(disable: 4201)   // nameless struct/union

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <XUtil/export.h>
#include <XUtil/String/xString.h>

//#include <strstream>

#include <atlbase.h>       // USES_CONVERSION

#pragma warning(default: 4201)

///////////////////////////////////////////////////////////////////////////////
// Debugging defines...
///////////////////////////////////////////////////////////////////////////////

#ifndef DEBUG_ONLY
#ifdef _DEBUG
#define DEBUG_ONLY(x)   x
#else
#define DEBUG_ONLY(x)
#endif
#endif

XUTIL_API BOOL IsExistFile( CxString strPathName );

XUTIL_API void MakeDirectory( CxString strDir );

XUTIL_API CxString HexToString( const BYTE *pBuffer, size_t iBytes );

XUTIL_API void StringToHex( const CxString &str, BYTE *pBuffer, size_t nBytes );

XUTIL_API CxString GetLastErrorMessage( DWORD dwLastError );

XUTIL_API CxString GetCurrentDirectory();

XUTIL_API CxString GetExecuteDirectory();

XUTIL_API CxString GetExecuteFile();

XUTIL_API CxString GetDateStamp();

XUTIL_API CxString GetTime();

XUTIL_API CxString ToHexC(BYTE c);

XUTIL_API CxString DumpData( const BYTE * const pData, size_t dataLength, size_t lineLength = 0 );

XUTIL_API CxString GetComputerName();

XUTIL_API CxString GetModuleFileName( HINSTANCE hModule = NULL );

XUTIL_API CxString GetUserName();

#pragma comment(lib, "Version.lib")
XUTIL_API CxString GetFileVersion();

XUTIL_API CxString RelativePathToAbsolutePath( CxString strRelativePath );

XUTIL_API void DeleteDirectory( CxString strDirectory );


#endif // __X_UTILS_H__