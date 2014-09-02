/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"

#include <process.h>

#include <XUtil/xThread.h>
#include <XUtil/xWin32Exception.h>

///////////////////////////////////////////////////////////////////////////////
// CxThread
///////////////////////////////////////////////////////////////////////////////

CxThread::CxThread() :
	m_hThread(NULL)
{

}
      
CxThread::~CxThread()
{
   if (m_hThread != NULL)
   {
      ::CloseHandle(m_hThread);
	  m_hThread = NULL;
   }
}

HANDLE CxThread::GetHandle() const
{
   return m_hThread;
}

void CxThread::Start()
{
	if (m_hThread == NULL)
	{
		unsigned int nThreadID = 0;
		m_hThread = (HANDLE)::_beginthreadex(0, 0, ThreadFunction, (void*)this, 0, &nThreadID);

		if (m_hThread == 0)
		{
			throw CxWin32Exception(_T("CxThread::Start() - _beginthreadex"), errno);
		}
	}
	else
	{
		throw CxException(_T("CxThread::Start()"), _T("Thread already running - you can only call Start() once!"));
	}
}

void CxThread::Wait() const
{
   if (!Wait(INFINITE))
   {
      throw CxException(_T("CxThread::Wait()"), _T("Unexpected timeout on infinite wait"));
   }
}

BOOL CxThread::Wait(DWORD dwTimeoutMillis) const
{
   BOOL bOK;

   if ( !m_hThread ) return TRUE;

   DWORD dwResult = ::WaitForSingleObject(m_hThread, dwTimeoutMillis);

   if (dwResult == WAIT_TIMEOUT)
   {
      bOK = FALSE;
   }
   else if (dwResult == WAIT_OBJECT_0)
   {
      bOK = TRUE;
   }
   else
   {
      throw CxWin32Exception(_T("CxThread::Wait() - WaitForSingleObject"), ::GetLastError());
   }
    
   return bOK;
}

unsigned int __stdcall CxThread::ThreadFunction(void *pV)
{
   DWORD result = 0;

   CxThread* pThis = (CxThread*)pV;
   
   if (pThis)
   {
      try
      {
         result = pThis->Run();
      }
      catch (...)
      {
      }
   }

   _endthreadex( 0 );

   return result;
}

void CxThread::Terminate(DWORD dwExitCode /*= 0*/)
{
   if (!::TerminateThread(m_hThread, dwExitCode))
   {
      throw CxWin32Exception(_T("CxThread::Terminate() - TerminateThread"), ::GetLastError());
   }
}