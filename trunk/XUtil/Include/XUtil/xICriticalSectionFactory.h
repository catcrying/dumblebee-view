#ifndef __X_ICRITICALSECTIONFACTORY_H__
#define __X_ICRITICALSECTIONFACTORY_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/export.h>
#include <XUtil/String/xString.h>

class CxCriticalSection;

///////////////////////////////////////////////////////////////////////////////
// ICriticalSectionFactory
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API ICriticalSectionFactory
{
   public :
   
      virtual ~ICriticalSectionFactory() {}

      virtual CxCriticalSection &GetCriticalSection(void *pKey) const = 0;

      virtual CxCriticalSection &GetCriticalSection(const CxString &strKey) const = 0;

      virtual CxCriticalSection &GetCriticalSection(const size_t nKey) const = 0;
};

#endif // __X_ICRITICALSECTIONFACTORY_H__