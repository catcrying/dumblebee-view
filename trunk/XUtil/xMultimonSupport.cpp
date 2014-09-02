/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "StdAfx.h"
#include <XUtil/DebugSupport/xDebug.h>
#include <XUtil/xMultimonSupport.h>
#include <vector>
#include <tchar.h>

class CxMMSupportData
{
public:
	std::vector<MONITORINFOEX>	m_vMonitorInfo;
};

BOOL CALLBACK CxMultimonSupport::_MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	CxMultimonSupport* pThis = (CxMultimonSupport*)dwData;
	return pThis->MonitorEnumProc( hMonitor, hdcMonitor, lprcMonitor );
}

CxMultimonSupport::CxMultimonSupport(void)
{
	m_pData = new CxMMSupportData();
	Refresh();
}

CxMultimonSupport::~CxMultimonSupport(void)
{
	delete m_pData;
}

void CxMultimonSupport::Refresh()
{
	m_nPrimaryMonitorIndex = -1;
	::SetRectEmpty( &m_rcVirtualScrn );
	m_pData->m_vMonitorInfo.clear();

	BOOL bOK = ::EnumDisplayMonitors( NULL, NULL, _MonitorEnumProc, (LPARAM)this );
}

BOOL CxMultimonSupport::MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor )
{
	MONITORINFOEX MonitorInfo; 
	MonitorInfo.cbSize = sizeof( MONITORINFOEX );

	if ( !::GetMonitorInfo( hMonitor, &MonitorInfo ) )
	{
		XTRACE( _T("CxMultimonSupport::MonitorEnumProc - GetMonitorInfo Error: %d\r\n"), GetLastError() );
		return TRUE;
	}

	if ( m_pData->m_vMonitorInfo.size() == 0 )
	{
		m_rcVirtualScrn = MonitorInfo.rcMonitor;
	}
	else
	{
		::UnionRect( &m_rcVirtualScrn, &m_rcVirtualScrn, &MonitorInfo.rcMonitor );
	}

	m_pData->m_vMonitorInfo.push_back( MonitorInfo );

	if ( MonitorInfo.dwFlags & MONITORINFOF_PRIMARY )
		m_nPrimaryMonitorIndex = (int)( m_pData->m_vMonitorInfo.size()-1 );

	return TRUE;
}

UINT CxMultimonSupport::GetMonitorCount()
{
	return (UINT)m_pData->m_vMonitorInfo.size();
}

SIZE CxMultimonSupport::GetMonitorResolution( UINT nIndex )
{
	SIZE szRes;
	szRes.cx = 0; szRes.cy = 0;
	if ( GetMonitorCount() <= nIndex )
	{
		return szRes;
	}

	MONITORINFOEX& MonitorInfo = m_pData->m_vMonitorInfo[nIndex];
	szRes.cx = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
	szRes.cy = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
	return szRes;
}

RECT CxMultimonSupport::GetMonitorPosition( UINT nIndex )
{
	RECT rcPos;
	rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
	if ( GetMonitorCount() <= nIndex )
	{
		return rcPos;
	}

	MONITORINFOEX& MonitorInfo = m_pData->m_vMonitorInfo[nIndex];
	rcPos = MonitorInfo.rcMonitor;
	return rcPos;
}

UINT CxMultimonSupport::GetPrimaryMonitorIndex()
{
	return m_nPrimaryMonitorIndex < 0 ? 0 : (UINT)m_nPrimaryMonitorIndex;
}
