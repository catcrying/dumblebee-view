// GdiPlusInitializer.cpp: implementation of the CGdiPlusInitializer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GdiPlusInitializer.h"

#if _MSC_VER < 1300
#include <AfxPriv.h>
typedef DWORD ULONG_PTR;
#endif

#include <GdiPlus.h>

#pragma comment(lib, "GdiPlus.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGdiPlusInitializer::CGdiPlusInitializer()
{
	m_bIsInit = false;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;

	Gdiplus::Status status = Gdiplus::GdiplusStartup(&m_dwGdiplusToken, &gdiplusStartupInput, NULL);

	if( status == Gdiplus::Ok )
	{
		m_bIsInit = true;
	}

	TRACE( _T("GdiPlus Initialize OK\r\n") );
}

CGdiPlusInitializer::~CGdiPlusInitializer()
{
	if ( m_dwGdiplusToken != 0 )
	{
		// Exit GDI+.
		Gdiplus::GdiplusShutdown(m_dwGdiplusToken);
	}

	m_bIsInit = false;

	TRACE( _T("GdiPlus Shutdown OK\r\n") );
}

bool CGdiPlusInitializer::IsInitialize()
{
	return m_bIsInit;
}

CGdiPlusInitializer	_GdiPlusInitializer;