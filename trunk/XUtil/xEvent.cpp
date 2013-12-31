#include "stdafx.h"

#include <XUtil/xEvent.h>
#include <XUtil/xWin32Exception.h>

///////////////////////////////////////////////////////////////////////////////
// Static helper methods
///////////////////////////////////////////////////////////////////////////////

static HANDLE Create(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName);

///////////////////////////////////////////////////////////////////////////////
// CxEvent
///////////////////////////////////////////////////////////////////////////////

CxEvent::CxEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState) :
	m_hEvent(Create(lpEventAttributes, bManualReset, bInitialState, 0))
{

}

CxEvent::CxEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpszName) :
	m_hEvent(Create(lpEventAttributes, bManualReset, bInitialState, lpszName))
{

}

CxEvent::~CxEvent()
{
	if (m_hEvent != NULL)
	{
	   ::CloseHandle(m_hEvent);
	   m_hEvent = NULL;
	}
}

HANDLE CxEvent::GetEvent() const
{
   return m_hEvent;
}

void CxEvent::Wait() const
{
   if (!Wait(INFINITE))
   {
      throw CxException(_T("CxEvent::Wait()"), _T("Unexpected timeout on infinite wait"));
   }
}

BOOL CxEvent::Wait(DWORD dwTimeoutMillis) const
{
   BOOL ok;

   DWORD result = ::WaitForSingleObject(m_hEvent, dwTimeoutMillis);

   if (result == WAIT_TIMEOUT)
   {
      ok = FALSE;
   }
   else if (result == WAIT_OBJECT_0)
   {
      ok = TRUE;
   }
   else
   {
      throw CxWin32Exception(_T("CxEvent::Wait() - WaitForSingleObject"), ::GetLastError());
   }
    
   return ok;
}

void CxEvent::Reset()
{
   if (!::ResetEvent(m_hEvent))
   {
      throw CxWin32Exception(_T("CxEvent::Reset()"), ::GetLastError());
   }
}

void CxEvent::Set()
{
   if (!::SetEvent(m_hEvent))
   {
      throw CxWin32Exception(_T("CxEvent::Set()"), ::GetLastError());
   }
}

void CxEvent::Pulse()
{
   if (!::PulseEvent(m_hEvent))
   {
      throw CxWin32Exception(_T("CxEvent::Pulse()"), ::GetLastError());
   }
}

///////////////////////////////////////////////////////////////////////////////
// Static helper methods
///////////////////////////////////////////////////////////////////////////////

static HANDLE Create(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpName)
{
   HANDLE hEvent = ::CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
   DWORD dwLastError = ::GetLastError();
   if ( dwLastError == ERROR_ALREADY_EXISTS )
   {
	   XTRACE( _T("CxEvent::Create: %s\r\n"), _T("ERROR_ALREADY_EXISTS") );
   }

   if (hEvent == NULL)
   {
      throw CxWin32Exception(_T("CxEvent::Create()"), dwLastError);
   }

   return hEvent;
}
