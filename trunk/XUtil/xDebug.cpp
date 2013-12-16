#include "stdafx.h"

#include <wtypes.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <XUtil/DebugSupport/xDebug.h>

#ifdef _DEBUG

BOOL XAssertFailedLine( LPCSTR lpszFileName, int nLine )
{
	MSG msg;
	BOOL bQuit = ::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);

	TCHAR pMsg[1024];
	TCHAR pModulePath[MAX_PATH];
	::GetModuleFileName( NULL, pModulePath, MAX_PATH );
	_sntprintf(pMsg, 1024, _T("Debug Assertion Failed!\n\nProgram: %s\nFile: %s\nLine: %d\n\n(Press Retry to debug the application)"),
		pModulePath, lpszFileName, nLine);

	int nResult = ::MessageBox(NULL, pMsg, _T("XUtil Debug Library"), MB_ABORTRETRYIGNORE | MB_ICONSTOP);
	if ( nResult == IDABORT )
	{
		exit(0);
	}
	if ( bQuit )
	{
		PostQuitMessage( (int)msg.wParam );
	}

	return ( nResult == IDRETRY );
}

#define E_TRACE_BUFFER_SIZE 1024

void XTrace( LPCSTR lpszFormat, ... )
{
	if ( lpszFormat == NULL )
	{
		return;
	}

	va_list argList;
	va_start(argList, lpszFormat);

	XTraceV( lpszFormat, argList );

	va_end(argList);
}

void XTraceV( LPCSTR lpszFormat, va_list argList )
{
	if ( lpszFormat == NULL )
	{
		return;
	}

	CHAR lptszBuffer[E_TRACE_BUFFER_SIZE];
	XVERIFY( _vsnprintf(lptszBuffer, E_TRACE_BUFFER_SIZE, lpszFormat, argList) <= E_TRACE_BUFFER_SIZE );

	::OutputDebugStringA( lptszBuffer );
}

void XTrace( LPCWSTR lpszFormat, ... )
{
	if ( lpszFormat == NULL )
	{
		return;
	}

	va_list argList;
	va_start(argList, lpszFormat);

	XTraceV( lpszFormat, argList );

	va_end(argList);
}

void XTraceV( LPCWSTR lpszFormat, va_list argList )
{
	if ( lpszFormat == NULL )
	{
		return;
	}

	WCHAR lptszBuffer[E_TRACE_BUFFER_SIZE];
	XVERIFY( _vsnwprintf(lptszBuffer, E_TRACE_BUFFER_SIZE, lpszFormat, argList) <= E_TRACE_BUFFER_SIZE );

	::OutputDebugStringW( lptszBuffer );
}

#endif _DEBUG

