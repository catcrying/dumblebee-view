#ifndef __X_THREAD_H__
#define __X_THREAD_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <XUtil/export.h>

///////////////////////////////////////////////////////////////////////////////
// CxThread
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxThread 
{
   public :
   
      CxThread();
      
      virtual ~CxThread();

      HANDLE GetHandle() const;

      void Wait() const;

      BOOL Wait(DWORD dwTimeoutMillis) const;

      void Start();

      void Terminate(DWORD dwExitCode = 0);

   private :

      virtual int Run() = 0;

	  //static DWORD WINAPI ThreadFunction(LPVOID pV );
      static unsigned int __stdcall ThreadFunction(void *pV);

      // No copies do not implement
      CxThread(const CxThread &rhs);
      CxThread &operator=(const CxThread &rhs);

	protected:
      HANDLE m_hThread;
};

#endif // __X_THREAD_H__