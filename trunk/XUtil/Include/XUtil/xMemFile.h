#ifndef __X_MEMFILE_H__
#define __X_MEMFILE_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xArchive.h>

class XUTIL_API CxMemFile : public CxArchive
{
public:
	int m_nExpandUnitSize;

	CxMemFile( int nInitMemSize = 0x400 );
	virtual ~CxMemFile();
	void Empty();
	int SetCurPos( int nPos );
	int GetSize();
	int GetCurPos();
	void* GetDataPointer();
	virtual BOOL Flush();
	virtual unsigned Read( void * lpBuf, unsigned nMax );
	virtual unsigned Write( const void * lpData, unsigned nSize );
	virtual BOOL IsLoading();
	virtual BOOL IsStoring();

	operator const void*() const;
	operator const char*() const;

protected:
	int m_nFilledMemSize;
	int m_nPosCursor;
	int m_nMemSize;
	char* m_pMem;

	BOOL ExpandMemory( int nExpandSize );

};

#endif // __X_MEMFILE_H__