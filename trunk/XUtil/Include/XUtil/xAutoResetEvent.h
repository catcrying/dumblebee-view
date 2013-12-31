#ifndef __X_AUTORESETEVENT_H__
#define __X_AUTORESETEVENT_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/export.h>
#include <XUtil/xEvent.h>

///////////////////////////////////////////////////////////////////////////////
// CxAutoResetEvent
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxAutoResetEvent : public CxEvent
{
   public :
   
      explicit CxAutoResetEvent(BOOL bInitialState = FALSE);
      
      explicit CxAutoResetEvent(LPCTSTR lpszName, BOOL bInitialState = FALSE);

	  virtual ~CxAutoResetEvent();

   private :
      // No copies do not implement
      CxAutoResetEvent(const CxAutoResetEvent &rhs);
      CxAutoResetEvent &operator=(const CxAutoResetEvent &rhs);
};

#endif // __X_AUTORESETEVENT_H__