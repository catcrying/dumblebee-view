#include "stdafx.h"

#include <XUtil/xAutoResetEvent.h>

///////////////////////////////////////////////////////////////////////////////
// CxAutoResetEvent
///////////////////////////////////////////////////////////////////////////////

CxAutoResetEvent::CxAutoResetEvent(BOOL bInitialState /* = FALSE */) : 
	CxEvent(0, FALSE, bInitialState)
{

}

CxAutoResetEvent::CxAutoResetEvent(LPCWSTR lpszName, BOOL bInitialState /* = FALSE */) : 
	CxEvent(0, FALSE, bInitialState, lpszName)
{
   
}

CxAutoResetEvent::CxAutoResetEvent(LPCSTR lpszName, BOOL bInitialState /* = FALSE */) : 
	CxEvent(0, FALSE, bInitialState, lpszName)
{
   
}

CxAutoResetEvent::~CxAutoResetEvent()
{

}