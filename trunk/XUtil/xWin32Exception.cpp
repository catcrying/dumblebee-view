#include "stdafx.h"

#include <XUtil/xWin32Exception.h>
#include <XUtil/xUtils.h>

///////////////////////////////////////////////////////////////////////////////
// CxWin32Exception
///////////////////////////////////////////////////////////////////////////////

CxWin32Exception::CxWin32Exception(const CxString &strWhere, DWORD dwError)
   :  CxException(strWhere, GetLastErrorMessage(dwError)),
      m_dwError(dwError)
{
}

DWORD CxWin32Exception::GetError() const 
{ 
   return m_dwError; 
}