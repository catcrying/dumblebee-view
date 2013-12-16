#ifndef __X_CRITICAL_SECTION_H__
#define __X_CRITICAL_SECTION_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/export.h>

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

class XUTIL_API CxCriticalSection 
{
   public:
      class XUTIL_API Owner
      {
         public:
            explicit Owner(CxCriticalSection &Crit);
            ~Owner();
      
         private:
            CxCriticalSection &m_Crit;

            // No copies do not implement
            Owner(const Owner &rhs);
            Owner &operator=(const Owner &rhs);
      };

      CxCriticalSection( DWORD dwSpinCount = 4000 );
      ~CxCriticalSection();

#if (_WIN32_WINNT >= 0x0400)
      BOOL TryEnter();
#endif
      void Enter();
      void Leave();

   private:
      CRITICAL_SECTION m_Crit;

      // No copies do not implement
      CxCriticalSection(const CxCriticalSection &rhs);
      CxCriticalSection &operator=(const CxCriticalSection &rhs);
};

#endif // __X_CRITICAL_SECTION_H__