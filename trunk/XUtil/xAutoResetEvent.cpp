/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"

#include <XUtil/xAutoResetEvent.h>

///////////////////////////////////////////////////////////////////////////////
// CxAutoResetEvent
///////////////////////////////////////////////////////////////////////////////

CxAutoResetEvent::CxAutoResetEvent(BOOL bInitialState /* = FALSE */) : 
	CxEvent(0, FALSE, bInitialState)
{

}

CxAutoResetEvent::CxAutoResetEvent(LPCTSTR lpszName, BOOL bInitialState /* = FALSE */) : 
	CxEvent(0, FALSE, bInitialState, lpszName)
{
   
}

CxAutoResetEvent::~CxAutoResetEvent()
{

}