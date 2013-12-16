#include "stdafx.h"

#include <XUtil/xCriticalSection.h>
#include <XUtil/xUtils.h>

///////////////////////////////////////////////////////////////////////////////
// CxCriticalSection
///////////////////////////////////////////////////////////////////////////////

CxCriticalSection::CxCriticalSection( DWORD dwSpinCount /*= 4000*/ )
{
#if (_WIN32_WINNT >= 0x0500)
	::InitializeCriticalSectionAndSpinCount( &m_Crit, dwSpinCount );
#else
	::InitializeCriticalSection( &m_Crit );
#endif
}
      
CxCriticalSection::~CxCriticalSection()
{
   ::DeleteCriticalSection(&m_Crit);
}

#if (_WIN32_WINNT >= 0x0400)
BOOL CxCriticalSection::TryEnter()
{
   return ::TryEnterCriticalSection(&m_Crit) ;
}
#endif

void CxCriticalSection::Enter()
{
   ::EnterCriticalSection(&m_Crit);
}

void CxCriticalSection::Leave()
{
   ::LeaveCriticalSection(&m_Crit);
}

///////////////////////////////////////////////////////////////////////////////
// CxCriticalSection::Owner
///////////////////////////////////////////////////////////////////////////////

CxCriticalSection::Owner::Owner(CxCriticalSection &Crit) :
	m_Crit(Crit)
{
   m_Crit.Enter();
}

CxCriticalSection::Owner::~Owner()
{
   m_Crit.Leave();
}