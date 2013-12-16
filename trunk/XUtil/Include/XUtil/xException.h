#ifndef __X_EXCEPTION_H__
#define __X_EXCEPTION_H__

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
// CxException
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxException
{
   public:
      CxException(const CxString &strWhere, const CxString &strMessage);
      virtual ~CxException();

      virtual CxString GetWhere() const;
      virtual CxString GetMessage() const; 

      void MessageBox(HWND hWnd = NULL) const; 

   protected:
      const CxString m_strWhere;
      const CxString m_strMessage;
};

#endif // __X_EXCEPTION_H__