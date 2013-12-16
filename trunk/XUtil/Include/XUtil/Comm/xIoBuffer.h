// IoBuffer.h: interface for the CxIoBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOBUFFER_H__2AF3C459_1B89_4EA7_8DE2_61265EF6A3B7__INCLUDED_)
#define AFX_IOBUFFER_H__2AF3C459_1B89_4EA7_8DE2_61265EF6A3B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <XUtil/export.h>
#include <wtypes.h>
#include <tchar.h>

class XUTIL_API CxIoBuffer  
{
protected:
	int				m_nSize;
	char*			m_pBuffer;
	int				m_nMaxBufferSize;
public:
	CxIoBuffer();
	virtual ~CxIoBuffer();

	BOOL Create( int nMaxBufferSize = 15*1024*1024 ); // 15M
	BOOL Add( char* pBuffer, int nBufSize );
	BOOL Remove( int nStart, int nEnd );
	int GetSize() const;
	void Clear();
	const char* GetBuffer() const;
};

#endif // !defined(AFX_IOBUFFER_H__2AF3C459_1B89_4EA7_8DE2_61265EF6A3B7__INCLUDED_)
