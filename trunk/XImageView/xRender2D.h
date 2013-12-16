#ifndef __XRENDER2D_H__
#define __XRENDER2D_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <wtypes.h>
#include <tchar.h>

class CxRender2D  
{
public:
	CxRender2D() {};
	virtual ~CxRender2D() {};

	virtual void Lock() = 0;
	virtual void Unlock() = 0;
	virtual BYTE* GetOffBuffer( int &nWidthBytes ) = 0;

	virtual BOOL EndDraw() = 0;

	virtual BOOL IsInit() const = 0;
	virtual BOOL Init( int nBitCnt ) = 0;
	virtual void Shutdown() = 0;

	virtual BOOL EndDraw( RECT rectRedraw ) = 0;

	virtual void SetDims( int nWidth, int nHeight ) = 0;
	virtual void GetDims( int& nWidth, int& nHeight ) const = 0;
	virtual int	 GetBufferBitsCnt() = 0;
	virtual void SetDC( HDC hDC ) = 0;

	virtual HDC GetInnerDC() = 0;
	virtual void ReleaseInnerDC() = 0;

	virtual void SetPalette( const BYTE * pPal ) = 0;
	virtual const BYTE* GetPalette() = 0;

};

#endif __XRENDER2D_H__
