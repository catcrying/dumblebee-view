#include "stdafx.h"

#include <malloc.h>
#include <XUtil/xMemFile.h>
#include <XUtil/DebugSupport/xDebug.h>

CxMemFile::CxMemFile( int nInitMemSize )
{
	if ( nInitMemSize < 0 ) nInitMemSize = 0;

	m_nExpandUnitSize = 0x400;

	m_nPosCursor = m_nFilledMemSize = 0;
	m_nMemSize = nInitMemSize;
	m_pMem = NULL;
	if ( nInitMemSize )
	{
		m_pMem = (char*)malloc(nInitMemSize);
		XASSERT( m_pMem );
	}
}

CxMemFile::~CxMemFile()
{
	m_nFilledMemSize = 0;
	m_nMemSize = 0;
	if ( m_pMem )
	{
		free(m_pMem);
		m_pMem = NULL;
	}
}

BOOL CxMemFile::ExpandMemory( int nExpandSize )
{
	if ( m_nMemSize >= nExpandSize )
	{
		return TRUE;
	}

	int nExSize = (nExpandSize / m_nExpandUnitSize) * m_nExpandUnitSize;
	if ( nExSize != nExpandSize )
	{
		nExSize += m_nExpandUnitSize;
	}

	if ( m_pMem )
	{
		m_pMem = (char*)realloc(m_pMem, nExSize);
		if ( m_pMem == NULL )
		{
			return FALSE;
		}
		m_nMemSize = nExSize;
	}
	else
	{
		m_pMem = (char*)malloc(nExpandSize);
		m_nMemSize = nExpandSize;
		m_nFilledMemSize = 0;
	}

	return TRUE;
}

void CxMemFile::Empty()
{
	m_nPosCursor = m_nFilledMemSize = 0;
}

int CxMemFile::SetCurPos( int nPos )
{
	if ( !ExpandMemory(nPos) )
	{
		return -1;
	}

	m_nPosCursor = nPos;
	return m_nPosCursor;
}

BOOL CxMemFile::Flush()
{
	return TRUE;
}

unsigned CxMemFile::Read( void * lpBuf, unsigned nMax )
{
	int nRead = m_nFilledMemSize - m_nPosCursor;
	if ( nRead <= 0 )
	{
		return 0;
	}

	if ( nRead > (signed)nMax )
	{
		nRead = nMax;
	}

	memcpy(lpBuf, m_pMem + m_nPosCursor, nRead);
	m_nPosCursor += nRead;

	return nRead;
}

unsigned CxMemFile::Write( const void * lpData, unsigned nSize )
{
	if ( m_nMemSize-m_nFilledMemSize < (signed)nSize )
	{
		if ( !ExpandMemory(m_nMemSize+nSize) ) return 0;
	}

	memcpy(m_pMem+m_nFilledMemSize, lpData, nSize);
	m_nPosCursor += nSize;
	if ( m_nFilledMemSize < m_nPosCursor )
	{
		m_nFilledMemSize = m_nPosCursor;
	}

	return nSize;
}

BOOL CxMemFile::IsLoading()
{
	return TRUE;
}

BOOL CxMemFile::IsStoring()
{
	return TRUE;
}

CxMemFile::operator const void*() const
{ 
	return m_pMem;
}

CxMemFile::operator const char*() const
{ 
	return m_pMem;
}

int CxMemFile::GetSize()
{
	return m_nFilledMemSize;
}

int CxMemFile::GetCurPos()
{
	return m_nPosCursor;
}

void * CxMemFile::GetDataPointer()
{
	return m_pMem;
}