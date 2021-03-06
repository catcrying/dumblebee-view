/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"

#include <XUtil/xException.h>

///////////////////////////////////////////////////////////////////////////////
// CxException
///////////////////////////////////////////////////////////////////////////////

CxException::CxException(const CxString &strWhere, const CxString &strMessage) :
	m_strWhere(strWhere), 
    m_strMessage(strMessage)
{
}

CxException::~CxException()
{
}

CxString CxException::GetWhere() const 
{ 
   return m_strWhere; 
}

CxString CxException::GetMessage() const 
{ 
   return m_strMessage; 
}

void CxException::MessageBox(HWND hWnd /* = NULL */) const 
{ 
   ::MessageBox(hWnd, GetMessage(), GetWhere(), MB_ICONSTOP); 
}
