#include	"StdAfx.h"
#include	<XUtil/xMmTimers.h>

CxMMTimers::CxMMTimers( UINT nResolution ) : m_nTimerRes(0), m_nTimerId(0), m_lParam(NULL)
{
	TIMECAPS	tc;

	if ( TIMERR_NOERROR == timeGetDevCaps(&tc,sizeof(TIMECAPS)) )
	{
		m_nTimerRes = min( max(tc.wPeriodMin, nResolution), tc.wPeriodMax );
		timeBeginPeriod( m_nTimerRes );
	}
	m_bTimerStart = FALSE;
}


CxMMTimers::CxMMTimers() : m_nTimerRes(0), m_nTimerId(0)
{
	CxMMTimers(1);
	m_bTimerStart = FALSE;
}


CxMMTimers::~CxMMTimers()
{
	StopTimer();
	if ( m_nTimerRes != 0 ) 
	{
		timeEndPeriod( m_nTimerRes );
		m_nTimerRes = 0;
	}
}

extern "C"
void CALLBACK _TimerProc(UINT id, UINT msg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	CxMMTimers*	pThis = (CxMMTimers *)dwUser;

	pThis->TimerProc();
}

BOOL CxMMTimers::StartTimer( UINT nPeriod, BOOL bOneShot )
{
	BOOL		bRes = FALSE;
	MMRESULT	mmResult;

	if ( m_bTimerStart && !bOneShot ) return FALSE;
	m_bTimerStart = TRUE;

	mmResult = timeSetEvent( nPeriod, m_nTimerRes, _TimerProc, (DWORD)this, bOneShot ? TIME_ONESHOT : TIME_PERIODIC );
	if ( mmResult != NULL )
	{
		m_nTimerId = (UINT)mmResult;
		bRes = TRUE;
	}

	return bRes;
}


BOOL CxMMTimers::StopTimer()
{
	MMRESULT	mmResult;

	m_bTimerStart = FALSE;

	mmResult = timeKillEvent( m_nTimerId );
	if ( TIMERR_NOERROR == mmResult )
		m_nTimerId = 0;

	return  (TIMERR_NOERROR == mmResult) ? TRUE : FALSE;

}

void CxMMTimers::TimerProc() 
{
	if ( m_FnTimerCallback != NULL )
	{
		(*m_FnTimerCallback)( m_lParam );
	}
}


void CxMMTimers::RegisterCallBack( TIMERCALLBACK fnTimerCallback, LPVOID lParam )
{
	m_FnTimerCallback = fnTimerCallback;
	m_lParam = lParam;
}
