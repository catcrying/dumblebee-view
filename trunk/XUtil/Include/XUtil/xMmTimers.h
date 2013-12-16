//////////////////////////////////////////////////////
//	Multimedia Timer								//
//////////////////////////////////////////////////////

#ifndef	___multimedia_timers___
#define	___multimedia_timers___

#include <wtypes.h>
#include <tchar.h>
#include <mmsystem.h>
#include <XUtil/export.h>

#pragma comment(lib, "Winmm.lib")

typedef BOOL (WINAPI *TIMERCALLBACK)(LPVOID);

class XUTIL_API CxMMTimers
{
public:

	//////////////////////////////////////////////////////////////////////////
	// callback
	void RegisterCallBack( TIMERCALLBACK, LPVOID lParam );

	//////////////////////////////////////////////////////////////////////////
	// construction
	CxMMTimers( UINT nResolution );
	CxMMTimers();

	virtual ~CxMMTimers();

	UINT	GetTimerRes() { return m_nTimerRes; };

	//////////////////////////////////////////////////////////////////////////
	// control
	BOOL	StartTimer( UINT nPeriod, BOOL bOneShot );
	BOOL	StopTimer();

	void	TimerProc();

protected:
	UINT	m_nTimerRes;
	UINT	m_nTimerId;

	BOOL	m_bTimerStart;
	LPVOID	m_lParam;
	TIMERCALLBACK	m_FnTimerCallback;
};


#endif