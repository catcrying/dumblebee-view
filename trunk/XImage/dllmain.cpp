/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */
 
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
		XTRACE( _T("XImaged.dll Initializing!\r\n") );
#else
		XTRACE( _T("XImage.dll Initializing!\r\n") );
#endif
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef _DEBUG
		XTRACE( _T("XImaged.dll Terminating!\r\n") );
#else
		XTRACE( _T("XImage.dll Terminating!\r\n") );
#endif
		break;
	}
	return TRUE;
}

