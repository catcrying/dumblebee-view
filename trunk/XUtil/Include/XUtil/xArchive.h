#ifndef __X_ARCHIVE_H__
#define __X_ARCHIVE_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/export.h>

class XUTIL_API CxArchive
{
public:
	virtual BOOL Flush() = 0;
	virtual unsigned Read( void * lpBuf, unsigned nMax ) = 0;
	virtual unsigned Write( const void * lpData, unsigned nSize ) = 0;
	virtual BOOL IsLoading() = 0;
	virtual BOOL IsStoring() = 0;
	virtual BOOL Lock( BOOL bWrite = TRUE ) { return FALSE; }
	virtual BOOL Unlock( BOOL bWrite = TRUE ) { return FALSE; }
	virtual ~CxArchive() { }
};

#endif // __X_ARCHIVE_H__
