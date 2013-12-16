#ifndef __X_MANUALRESETEVENT_H__
#define __X_MANUALRESETEVENT_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xEvent.h>

///////////////////////////////////////////////////////////////////////////////
// CxManualResetEvent
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxManualResetEvent : public CxEvent
{
   public:
      explicit CxManualResetEvent(BOOL bInitialState = FALSE);
      explicit CxManualResetEvent(const CxString &strName, BOOL bInitialState = FALSE);

   private:
      // No copies do not implement
      CxManualResetEvent(const CxManualResetEvent &rhs);
      CxManualResetEvent &operator=(const CxManualResetEvent &rhs);
};

#endif // __X_MANUALRESETEVENT_H__