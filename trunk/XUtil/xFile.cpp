#include "stdafx.h"

#include <malloc.h>

#include <XUtil/xFile.h>
#include <XUtil/xIo.h>
#include <XUtil/String/xString.h>

BOOL CxFile::IsOpen()
{
	return (m_hFile != NULL ? TRUE : FALSE);
}

DWORD CxFile::SeekToEnd()
{
	return Seek(0, CxFile::end);
}

void CxFile::SeekToBegin()
{
	Seek(0, CxFile::begin);
}

CxFile::CxFile()
{
	m_hFile = NULL;
	m_dwAccess = 0;
	m_bCloseOnDelete = FALSE;
	m_bLoading = TRUE;
}

CxFile::CxFile(const CxFile& op)
{
	m_hFile = op.m_hFile;
	m_dwAccess = op.m_dwAccess;
	m_bCloseOnDelete = FALSE;
	m_bLoading = op.m_bLoading;
}

CxFile::CxFile(HANDLE hFile)
{
	m_hFile = hFile;
	m_dwAccess = 0;
	m_bCloseOnDelete = FALSE;
	m_bLoading = TRUE;
}

CxFile::CxFile(LPCTSTR pStrFileName)
{
	m_hFile = ::CreateFile(pStrFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	m_bCloseOnDelete = TRUE;
}

CxFile::CxFile( LPCTSTR lpszFileName, unsigned int nOpenFlags )
{
	m_hFile = NULL;
	m_dwAccess = 0;
	m_bCloseOnDelete = FALSE;

	Open(lpszFileName, nOpenFlags);
}

CxFile::~CxFile()
{
	Close();
}

BOOL CxFile::IsLoading()
{
	if ( !IsOpen() )
	{
		return FALSE;
	}

	if ( !(m_dwAccess & GENERIC_READ) )
	{
		return FALSE;
	}

	return m_bLoading;
}

BOOL CxFile::IsStoring()
{
	if ( !IsOpen() )
	{
		return FALSE;
	}

	if ( !(m_dwAccess & GENERIC_WRITE) )
	{
		return FALSE;
	}

	return !m_bLoading;
}

DWORD CxFile::GetPosition()
{
	if ( !IsOpen() )
	{
		return (DWORD)-1;
	}

	XASSERT( m_hFile );
	return ::SetFilePointer(m_hFile, 0, NULL, current);
}

BOOL CxFile::Flush()
{
	if ( !IsOpen() )
	{
		return FALSE;
	}

	return ::FlushFileBuffers(m_hFile);
}

unsigned int CxFile::Read(void* lpBuf, unsigned int nMax)
{
	if (nMax == 0) return 0;

	XASSERT(m_hFile);
	XASSERT(lpBuf);

	DWORD dwRead;
	if (::ReadFile(m_hFile, lpBuf, nMax, &dwRead, NULL))
	{
		return (unsigned int)dwRead;
	}

	return 0;
}

unsigned int CxFile::Write(const void* lpData, unsigned int nSize)
{
	if (nSize == 0) return 0;

	XASSERT(m_hFile);
	XASSERT(lpData);

	DWORD nWritten;
	if (::WriteFile(m_hFile, lpData, nSize, &nWritten, NULL))
	{
		return nWritten;
	}

	return 0;
}

BOOL CxFile::Open(LPCTSTR lpszFileName, unsigned int nOpenFlags)
{
	XASSERT(lpszFileName);
	XASSERT((nOpenFlags&typeText) == 0);

	// CxFile objects are always binary
	nOpenFlags &= ~(unsigned int)typeBinary;

	Close();

	// map read/write mode
	XASSERT( (modeRead|modeWrite|modeReadWrite) == 0x0003 );
	m_dwAccess = 0;
	m_bLoading = TRUE;
	switch ( nOpenFlags & 0x0003 )
	{
	case modeRead:
		m_dwAccess = GENERIC_READ;
		break;
	case modeWrite:
		m_dwAccess = GENERIC_WRITE;
		m_bLoading = FALSE;
		break;
	case modeReadWrite:
		m_dwAccess = GENERIC_RDWR;
		m_bLoading = FALSE;
		break;
	default:
		XASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD dwShareMode = 0;
	switch ( nOpenFlags & 0x0070 )
	{
	default:
		XASSERT(FALSE);
	case shareCompat:
	case shareExclusive:
		dwShareMode = 0;
		break;
	case shareDenyWrite:
		dwShareMode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		dwShareMode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		dwShareMode = FILE_SHARE_WRITE | FILE_SHARE_READ;
		break;
	}

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = ( (nOpenFlags & modeNoInherit) == NULL );

	// map creation flags
	DWORD dwCreateFlag;
	if ( nOpenFlags & modeCreate )
	{
		if ( nOpenFlags & modeNoTruncate )
		{
			dwCreateFlag = OPEN_ALWAYS;
		}
		else
		{
			dwCreateFlag = CREATE_ALWAYS;
		}
	}
	else
	{
		dwCreateFlag = OPEN_EXISTING;
	}

	// attempt file creation
	HANDLE hFile = ::CreateFile(lpszFileName, m_dwAccess, dwShareMode, &sa,
		dwCreateFlag, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) return FALSE;

	m_hFile = hFile;
	m_bCloseOnDelete = TRUE;

	return TRUE;
}

void CxFile::Close()
{
	Flush();

	if ( m_bCloseOnDelete )
	{
		if (m_hFile) ::CloseHandle(m_hFile);
	}

	m_hFile = NULL;
	m_dwAccess = 0;
	m_bCloseOnDelete = FALSE;
}

long CxFile::Seek( long lOff, unsigned int nFrom )
{
	XASSERT( nFrom==begin || nFrom==end || nFrom==current );
	return ::SetFilePointer(m_hFile, lOff, NULL, (DWORD)nFrom);
}

BOOL CxFile::SetLength( DWORD dwNewLen )
{
	XASSERT(m_hFile);
	if ( (DWORD)Seek((long)dwNewLen, (unsigned int)begin) != dwNewLen ) return FALSE;
	return ::SetEndOfFile(m_hFile);
}

DWORD CxFile::GetLength()
{
	if ( !IsOpen() ) return (DWORD)-1;

	long lCur = Seek(0L, current);
	DWORD dwLen = SeekToEnd();
	XVERIFY( lCur == Seek(lCur, begin) );
	return dwLen;
}
