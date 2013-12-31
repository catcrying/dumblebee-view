#ifndef __X_EVENT_H__
#define __X_EVENT_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <XUtil/export.h>
#include <XUtil/String/xString.h>

///////////////////////////////////////////////////////////////////////////////
// CxEvent
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxEvent 
{
   public:
      CxEvent(LPSECURITY_ATTRIBUTES lpSecurityAttributes, BOOL bManualReset, BOOL bInitialState);
	  CxEvent(LPSECURITY_ATTRIBUTES lpSecurityAttributes, BOOL bManualReset, BOOL bInitialState, LPCTSTR lpszName);

      virtual ~CxEvent();

      HANDLE GetEvent() const;

      void Wait() const;
      BOOL Wait(DWORD dwTimeoutMillis) const;

      void Reset();
      void Set();

      void Pulse();

   private:
      HANDLE m_hEvent;

      // No copies do not implement
      CxEvent(const CxEvent &rhs);
      CxEvent &operator=(const CxEvent &rhs);
};

#endif // __X_EVENT_H__