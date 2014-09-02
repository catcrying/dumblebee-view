/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"

#include <XUtil/xSharedCriticalSection.h>
#include <XUtil/xCriticalSection.h>
#include <XUtil/xException.h>
#include <XUtil/xUtils.h>

///////////////////////////////////////////////////////////////////////////////
// CxSharedCriticalSection
///////////////////////////////////////////////////////////////////////////////

CxSharedCriticalSection::CxSharedCriticalSection(const size_t nNumLocks, const size_t nMultiplier /* = 37 */) : 
	m_nNumLocks(nNumLocks),
    m_nMultiplier(nMultiplier),
    m_pCrits(new CxCriticalSection[nNumLocks])
{
}
      
CxSharedCriticalSection::~CxSharedCriticalSection()
{
   delete [] m_pCrits;
   m_pCrits = NULL;
}

CxCriticalSection &CxSharedCriticalSection::GetCriticalSection(void *pKey) const
{
   return GetCriticalSection(Hash(pKey));
}

CxCriticalSection &CxSharedCriticalSection::GetCriticalSection(const CxString &strKey) const
{
   return GetCriticalSection(Hash(strKey));
}

CxCriticalSection &CxSharedCriticalSection::GetCriticalSection(const size_t nKey) const
{
   if (nKey > m_nNumLocks)
   {
      throw CxException(_T("CxSharedCriticalSection::GetCriticalSection()"), _T("Key too large"));
   }

   return m_pCrits[nKey];
}

size_t CxSharedCriticalSection::Hash(void *pKey) const
{
   size_t nHash = reinterpret_cast<size_t>(pKey);

   return nHash % m_nNumLocks;
}

size_t CxSharedCriticalSection::Hash(const CxString &strKey) const
{
   size_t nHash = 0;

   const unsigned char *p = reinterpret_cast<const unsigned char *>((LPCTSTR)strKey);

   while (*p)
   {
      nHash = m_nMultiplier * nHash + *p;

      p++;
   }

   return nHash % m_nNumLocks;
}
   
///////////////////////////////////////////////////////////////////////////////
// CxInstrumentedSharedCriticalSection
///////////////////////////////////////////////////////////////////////////////

CxInstrumentedSharedCriticalSection::CxInstrumentedSharedCriticalSection( const size_t nNumLocks, const size_t nMultiplier) :
	CxSharedCriticalSection(nNumLocks, nMultiplier),
    m_pStats(new size_t[nNumLocks])
{
   for (size_t i = 0; i < m_nNumLocks; ++i)
   {
      m_pStats[i] = 0;
   }
}     

CxInstrumentedSharedCriticalSection::~CxInstrumentedSharedCriticalSection()
{
   size_t max = 0;
   size_t min = -1;
   size_t ave = 0;

   for (size_t i = 0; i < m_nNumLocks; ++i)
   {
      const size_t stat = m_pStats[i];
      
      if (stat > max)
      {
         max = stat;
      }

      if (stat < min)
      {
         min = stat;
      }

      ave += stat;

	  XTRACE( _T("%d\t-%d\r\n"), i, stat );
   }

   ave /= m_nNumLocks;

   XTRACE( _T("Min: %d "), min );
   XTRACE( _T("Max: %d "), max );
   XTRACE( _T("Ave: %d\r\n"), ave );

   delete [] m_pStats;
   m_pStats = NULL;
}

CxCriticalSection &CxInstrumentedSharedCriticalSection::GetCriticalSection(const size_t nKey) const
{
   CxCriticalSection &Crit = CxSharedCriticalSection::GetCriticalSection(nKey);

   m_pStats[nKey]++;

   return Crit;
}