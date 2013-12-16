// IoBuffer.cpp: implementation of the CxIoBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XUtil/Comm/xIoBuffer.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxIoBuffer::CxIoBuffer() :
	m_nSize(0), m_pBuffer(NULL), m_nMaxBufferSize(0)
{
}

CxIoBuffer::~CxIoBuffer()
{
	if ( m_pBuffer )
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

int CxIoBuffer::GetSize() const
{ 
	return m_nSize;
}

void CxIoBuffer::Clear()
{
	m_nSize = 0;
}

const char* CxIoBuffer::GetBuffer() const
{ 
	return m_pBuffer;
}

BOOL CxIoBuffer::Create( int nMaxBufferSize /*= 15*1024*1024*/ )
{
	if ( m_pBuffer )
		return TRUE;

	m_pBuffer = new char[nMaxBufferSize];
	if ( !m_pBuffer )
		return FALSE;

	m_nMaxBufferSize = nMaxBufferSize;

	return TRUE;
}

BOOL CxIoBuffer::Add( char* pBuffer, int nBufSize )
{
	if ( m_nSize + nBufSize > m_nMaxBufferSize-1 )
		return FALSE;
	::CopyMemory( m_pBuffer+m_nSize, pBuffer, nBufSize );

	m_nSize += nBufSize;

	return TRUE;
}

BOOL CxIoBuffer::Remove( int nStart, int nEnd )
{
	if ( nStart < 0 || nEnd >= m_nSize || nStart == nEnd )
		return FALSE;

	if ( m_nSize != nEnd+1 )
	{
		::MoveMemory( m_pBuffer+nStart, m_pBuffer+nEnd+1, m_nSize-nEnd-1 );
	}
	m_nSize -= (nEnd-nStart+1);

	return TRUE;
}
