/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#ifndef __XDIRECTDIB_H__
#define __XDIRECTDIB_H__

#include <vfw.h>
#include "xRender2D.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define NUM_SYS_COLORS 19

class CxDirectDIB : public CxRender2D
{
protected:
	BOOL		m_bSystemcolorsSaved;
	HGDIOBJ		m_hPreviouslySelectedGDIObj;
	int			m_pnSysPalIndices[NUM_SYS_COLORS];
	int			m_pnOldSysColors[NUM_SYS_COLORS];

	HDC			m_hDC;

	HBITMAP		m_hDIBSection;
	HDC			m_hdcDIBSection;
	HDRAWDIB	m_hDrawDib;

	BYTE*		m_pDIBBase;
	BYTE*		m_pOffBuffer;

	HPALETTE	m_hPal;
	HPALETTE	m_hOldPal;
	BOOL		m_bPalettized;
	unsigned	m_pn8to24table[256];	// quake's palette

	BYTE		m_pPalette[256*4];
	BOOL		m_bSetPalette;
	int			m_nWidth, m_nHeight;
	int			m_nBitCnt;
	int			m_nPitch;

	BOOL		m_bInit;

	typedef struct tagDIBinfo
	{
		BITMAPINFOHEADER	header;
		RGBQUAD				acolors[256];
	} DIBinfo;

	typedef struct tagIdentityPalette
	{
		WORD palVersion;
		WORD palNumEntries;
		PALETTEENTRY palEntries[256];
	} IdentityPalette;

	IdentityPalette m_IdentityPalette;
	DIBinfo			m_dibheader;

	void	SaveSystemColors();
	void	RestoreSystemColors();
	int		GetWidthBytes( int nCx, int nBitCount );
	
public:
	CxDirectDIB();
	virtual ~CxDirectDIB();

	virtual void Lock() {}
	virtual void Unlock() {}

	virtual BYTE* GetOffBuffer(int &nWidthBytes) { nWidthBytes = m_nPitch; return m_pOffBuffer; }
	virtual BOOL EndDraw();
	virtual BOOL EndDraw( RECT rectRedraw );

	virtual BOOL IsInit() const;
	virtual BOOL Init( int nBitCnt );
	virtual void Shutdown();

	virtual void SetDims( int nWidth, int nHeight ) { m_nWidth = nWidth, m_nHeight = nHeight; }
	virtual void GetDims( int& nWidth, int& nHeight ) const { nWidth = m_nWidth, nHeight = m_nHeight; }
	virtual int  GetBufferBitsCnt() { return m_nBitCnt; }
	virtual void SetDC( HDC hDC ) { m_hDC = hDC; }

	virtual HDC GetInnerDC() { return m_hdcDIBSection; }
	virtual void ReleaseInnerDC() {}
	HBITMAP GetInnerBitmap() { return m_hDIBSection; }

	virtual void SetPalette( const BYTE * pPal );
	virtual const BYTE* GetPalette();
};

inline int CxDirectDIB::GetWidthBytes( int nCx, int nBitCount )
{
	DWORD dwBytes = nCx * nBitCount;
	return ((dwBytes & 0x001f) ? (dwBytes >> 5) + 1 : dwBytes >> 5) << 2;
}

inline void CxDirectDIB::RestoreSystemColors()
{
    ::SetSystemPaletteUse( m_hDC, SYSPAL_STATIC );
    ::SetSysColors( NUM_SYS_COLORS, (CONST INT*)m_pnSysPalIndices, (const COLORREF*)m_pnOldSysColors );
}

inline void CxDirectDIB::SaveSystemColors()
{
	int i;
	for ( i = 0 ; i < NUM_SYS_COLORS ; i++ )
		m_pnOldSysColors[i] = ::GetSysColor( m_pnSysPalIndices[i] );
}

#endif __XDIRECTDIB_H__
