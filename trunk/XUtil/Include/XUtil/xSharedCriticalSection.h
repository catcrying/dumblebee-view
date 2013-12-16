#ifndef __X_SHAREDCRITICALSECTION_H__
#define __X_SHAREDCRITICALSECTION_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xICriticalSectionFactory.h>

///////////////////////////////////////////////////////////////////////////////
// CxSharedCriticalSection
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxSharedCriticalSection : public ICriticalSectionFactory
{
   public:
      CxSharedCriticalSection(const size_t nNumLocks, const size_t nMultiplier = 37);
      
      virtual ~CxSharedCriticalSection();

      virtual CxCriticalSection &GetCriticalSection(void *pKey) const;
      virtual CxCriticalSection &GetCriticalSection(const CxString &strKey) const;
      virtual CxCriticalSection &GetCriticalSection(const size_t nKey) const;

   protected:

      size_t Hash(void *pKey) const;
      size_t Hash(const CxString &strKey) const;

      const size_t m_nNumLocks;
      const size_t m_nMultiplier;

   private:
      CxCriticalSection *m_pCrits;

      // No copies do not implement
      CxSharedCriticalSection(const CxSharedCriticalSection &rhs);
      CxSharedCriticalSection &operator=(const CxSharedCriticalSection &rhs);
};

///////////////////////////////////////////////////////////////////////////////
// CxInstrumentedSharedCriticalSection
///////////////////////////////////////////////////////////////////////////////

class XUTIL_API CxInstrumentedSharedCriticalSection : public CxSharedCriticalSection 
{
   public:
   
      CxInstrumentedSharedCriticalSection(const size_t nNumLocks, const size_t nMultiplier = 37);
      
      virtual ~CxInstrumentedSharedCriticalSection();

      virtual CxCriticalSection &GetCriticalSection(const size_t nKey) const;

   private:

      size_t *m_pStats;

      // No copies do not implement
      CxInstrumentedSharedCriticalSection(const CxInstrumentedSharedCriticalSection &rhs);
      CxInstrumentedSharedCriticalSection &operator=(const CxInstrumentedSharedCriticalSection &rhs);
};

#endif // __X_SHAREDCRITICALSECTION_H__