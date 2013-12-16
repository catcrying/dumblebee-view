#ifndef __X_SYSTEMINFO_H__
#define __X_SYSTEMINFO_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

///////////////////////////////////////////////////////////////////////////////
// CxSystemInfo
///////////////////////////////////////////////////////////////////////////////

class CxSystemInfo : public SYSTEM_INFO 
{
   public:
      
      CxSystemInfo() 
      { 
         ::GetSystemInfo(this); 
      }
};

#endif // __X_SYSTEMINFO_H__