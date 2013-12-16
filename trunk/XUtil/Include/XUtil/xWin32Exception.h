#ifndef __X_WIN32EXCEPTION_H__
#define __X_WIN32EXCEPTION_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xException.h>

///////////////////////////////////////////////////////////////////////////////
// CxWin32Exception
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxWin32Exception : public CxException
{
   public : 

      CxWin32Exception(
         const CxString &strWhere, 
         DWORD dwError);

      DWORD GetError() const;

   protected :
      
      DWORD m_dwError;
};

#endif // __X_WIN32EXCEPTION_H__