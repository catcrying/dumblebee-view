#pragma once

#include <XUtil/export.h>
#include <wtypes.h>
#include <tchar.h>

class CxMMSupportData;
class XUTIL_API CxMultimonSupport
{
protected:
	CxMMSupportData*			m_pData;
	int							m_nPrimaryMonitorIndex;

	RECT						m_rcVirtualScrn;
public:
	UINT GetMonitorCount();
	SIZE GetMonitorResolution( UINT nIndex );
	RECT GetMonitorPosition( UINT nIndex );
	UINT GetPrimaryMonitorIndex();

	RECT GetVirtualScreen();

	void Refresh();

protected:
	static BOOL CALLBACK _MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData );
	BOOL MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor );

public:
	CxMultimonSupport(void);
	~CxMultimonSupport(void);
};

inline RECT CxMultimonSupport::GetVirtualScreen()
	{
		return m_rcVirtualScrn;
	}