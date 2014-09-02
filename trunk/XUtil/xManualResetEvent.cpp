/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"

#include <XUtil/xManualResetEvent.h>

///////////////////////////////////////////////////////////////////////////////
// CxManualResetEvent
///////////////////////////////////////////////////////////////////////////////

CxManualResetEvent::CxManualResetEvent(BOOL bInitialState /* = FALSE */) :
	CxEvent(0, TRUE, bInitialState)
{
}

CxManualResetEvent::CxManualResetEvent(const CxString &strName, BOOL bInitialState /* = FALSE */) :
	CxEvent(0, TRUE, bInitialState, strName)
{
}