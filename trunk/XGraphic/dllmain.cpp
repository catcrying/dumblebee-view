// dllmain.cpp : DLL 응용 프로그램의 진입점을 정의합니다.
#include "stdafx.h"
#include <tchar.h>
#include <XUtil/DebugSupport/xDebug.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
		XTRACE( _T("XGraphicd.dll Initializing!\r\n") );
#else
		XTRACE( _T("XGraphic.dll Initializing!\r\n") );
#endif
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		XTRACE( _T("XGraphicd.dll Terminating!\r\n") );
#else
		XTRACE( _T("XGraphic.dll Terminating!\r\n") );
#endif
		break;
		break;
	}
	return TRUE;
}

