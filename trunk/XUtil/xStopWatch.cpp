// xStopWatch.cpp: implementation of the CxStopWatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XUtil/xStopWatch.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxStopWatch::CxStopWatch()
{
	m_ltIdleTimeBegin.QuadPart = 0;
	m_ltIdleTimeEnd.QuadPart = 0;
	m_dIdleTime = 0.0;
	m_dOffsetTime = 0.0;
	m_bPaused = FALSE;
	m_bStarted = FALSE;
}

CxStopWatch::~CxStopWatch()
{

}

void CxStopWatch::Begin( double dOffsetTime/*=0.0*/ )
{
	m_dOffsetTime = dOffsetTime;
	m_dIdleTime = 0.0;
	QueryPerformanceCounter( &m_ltLastTime );
	m_bStarted = TRUE;
}

void CxStopWatch::Stop()
{
	QueryPerformanceCounter( &m_ltCurTime );
	m_bStarted = FALSE;
}

void CxStopWatch::Pause()
{
	if ( !m_bStarted || m_bPaused ) return;
	QueryPerformanceCounter( &m_ltIdleTimeBegin );

	m_bPaused = TRUE;
}

void CxStopWatch::Resume()
{
	if ( !m_bStarted || !m_bPaused ) return;
	QueryPerformanceCounter( &m_ltIdleTimeEnd );

	QueryPerformanceFrequency( &m_ltFreq );
	m_dIdleTime += ((double)(m_ltIdleTimeEnd.QuadPart - m_ltIdleTimeBegin.QuadPart))/((double)m_ltFreq.QuadPart);

	m_bPaused = FALSE;
}

BOOL CxStopWatch::IsStarted()
{
	return m_bStarted;
}

double CxStopWatch::GetElapsedTime()
{
	if ( m_bStarted )
		QueryPerformanceCounter( &m_ltCurTime );

	QueryPerformanceFrequency( &m_ltFreq );
	
	if ( m_bPaused )
	{
		QueryPerformanceCounter( &m_ltIdleTimeEnd );
		m_dIdleTime += ((double)(m_ltIdleTimeEnd.QuadPart - m_ltIdleTimeBegin.QuadPart))/((double)m_ltFreq.QuadPart);
		QueryPerformanceCounter( &m_ltIdleTimeBegin );
	}
	
	double dElapsedTime = ((double)(m_ltCurTime.QuadPart - m_ltLastTime.QuadPart))/((double)m_ltFreq.QuadPart);

	return 1000.0 * (dElapsedTime - m_dIdleTime + m_dOffsetTime);
}