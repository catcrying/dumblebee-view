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