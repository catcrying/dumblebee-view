/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE XImageViewDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
#ifdef _UNICODE
		TRACE0("XImageViewud.dll Initializing!\n");
#else
		TRACE0("XImageViewd.dll Initializing!\n");
#endif
		
		// 확장 DLL을 한 번만 초기화합니다.
		if (!AfxInitExtensionModule(XImageViewDLL, hInstance))
			return 0;

		// 이 DLL을 리소스 체인에 삽입합니다.
		// 참고: 이 확장 DLL이 MFC 응용 프로그램이
		//  아닌 ActiveX 컨트롤과 같은 MFC 기본 DLL에
		//  의해 명시적으로 링크되어 있는 경우에는
		//  DllMain에서 이 줄을 제거하고, 제거한 줄은 이 확장 DLL에서
		//  내보낸 별도의 함수에 추가합니다.
		//  그런 다음 이 확장 DLL을 사용하는 기본 DLL은
		//  해당 함수를 명시적으로 호출하여 이 확장 DLL을 추가해야 합니다.
		//  그렇지 않으면 CDynLinkLibrary 개체가
		//  기본 DLL의 리소스 체인에 추가되지 않으므로
		//  심각한 문제가 발생합니다.

		new CDynLinkLibrary(XImageViewDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
#ifdef _UNICODE
		TRACE0("XImageViewud.dll Terminating!\n");
#else
		TRACE0("XImageViewd.dll Terminating!\n");
#endif

		// 소멸자가 호출되기 전에 라이브러리를 종료합니다.
		AfxTermExtensionModule(XImageViewDLL);
	}
	return 1;   // 확인
}

HMODULE GetResourceHandle() { return XImageViewDLL.hModule; }