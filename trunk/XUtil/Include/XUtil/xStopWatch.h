// xStopWatch.h: interface for the CxStopWatch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STOPWATCH_H__D7F54916_3A87_48A1_B9DC_8B8BA3B67AA6__INCLUDED_)
#define AFX_STOPWATCH_H__D7F54916_3A87_48A1_B9DC_8B8BA3B67AA6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/export.h>

class XUTIL_API CxStopWatch  
{
private:
	LARGE_INTEGER m_ltCurTime, m_ltLastTime, m_ltIdleTimeBegin, m_ltIdleTimeEnd;
	LARGE_INTEGER m_ltFreq; 

	double	m_dIdleTime;
	BOOL	m_bStarted;
	BOOL	m_bPaused;
	double	m_dOffsetTime;
public:
	CxStopWatch();
	virtual ~CxStopWatch();

	//////////////////////////////////////////////////////////////////////////
	// control
	void	Begin( double dOffsetTime=0.0 );
	void	Stop();
	void	Pause();
	void	Resume();

	BOOL	IsStarted();

	//////////////////////////////////////////////////////////////////////////
	// getting time
	double	GetElapsedTime();

};

#endif // !defined(AFX_STOPWATCH_H__D7F54916_3A87_48A1_B9DC_8B8BA3B67AA6__INCLUDED_)
