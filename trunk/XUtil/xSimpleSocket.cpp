#include "stdafx.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <XUtil/Comm/xSimpleSocket.h>
#include <XUtil/DebugSupport/xDebug.h>

#include <wtypes.h>
#include <tchar.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void WINAPI XSimpleSocketInitialize()
{
	OSVERSIONINFO verInfo = {0};
	WSADATA WSAData; 

	verInfo.dwOSVersionInfoSize = sizeof(verInfo);
	GetVersionEx(&verInfo);
	if ( verInfo.dwPlatformId  == VER_PLATFORM_WIN32_WINDOWS ) 
	{
		XTRACE( _T("Please run this application only on NT, thank you\r\n") );
		return;
	}

	int nRet;
	if ( (nRet = WSAStartup(MAKEWORD(2,2), &WSAData)) != 0 ) 
	{
		XTRACE( _T("WSAStartup() failed: %d\r\n"), nRet );
		return ;
	}
}

void WINAPI XSimpleSocketTerminate()
{
	WSACleanup();
}
