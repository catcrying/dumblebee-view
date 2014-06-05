#include "stdafx.h"
#include "xDirectDIB.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef _WIN32
#  error You should not be trying to compile this file on this platform
#endif

#pragma comment(lib, "vfw32.lib")

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxDirectDIB::CxDirectDIB()
{
	int pnSysPalIndices[] =
	{
		COLOR_ACTIVEBORDER,
		COLOR_ACTIVECAPTION,
		COLOR_APPWORKSPACE,
		COLOR_BACKGROUND,
		COLOR_BTNFACE,
		COLOR_BTNSHADOW,
		COLOR_BTNTEXT,
		COLOR_CAPTIONTEXT,
		COLOR_GRAYTEXT,
		COLOR_HIGHLIGHT,
		COLOR_HIGHLIGHTTEXT,
		COLOR_INACTIVEBORDER,
		COLOR_INACTIVECAPTION,
		COLOR_MENU,
		COLOR_MENUTEXT,
		COLOR_SCROLLBAR,
		COLOR_WINDOW,
		COLOR_WINDOWFRAME,
		COLOR_WINDOWTEXT
	};

	memcpy( m_pnSysPalIndices, pnSysPalIndices, sizeof(pnSysPalIndices) );

	m_bPalettized			= FALSE;
	m_bSystemcolorsSaved	= FALSE;
	m_hDC					= NULL;
	m_hdcDIBSection			= NULL;
	m_hDIBSection			= NULL;
	m_hOldPal				= NULL;
	m_hPal					= NULL;
	m_hPreviouslySelectedGDIObj = NULL;
	m_nWidth				= 0;
	m_nHeight				= 0;
	m_nBitCnt				= 0;
	m_pDIBBase				= NULL;
	m_pOffBuffer			= NULL;
	m_nPitch				= 0;
	m_hDrawDib				= NULL;
	m_bSetPalette = FALSE;
	m_bInit					= FALSE;

}

CxDirectDIB::~CxDirectDIB()
{
	m_bPalettized			= FALSE;
	m_bSystemcolorsSaved	= FALSE;
	m_hDC					= NULL;
	m_hdcDIBSection			= NULL;
	m_hDIBSection			= NULL;
	m_hOldPal				= NULL;
	m_hPal					= NULL;
	m_hPreviouslySelectedGDIObj = NULL;
	m_nWidth				= 0;
	m_nHeight				= 0;
	m_pDIBBase				= NULL;
	m_pOffBuffer			= NULL;
	m_nPitch				= 0;
}

BOOL CxDirectDIB::IsInit() const
{
	return m_bInit;
}

BOOL CxDirectDIB::Init( int nBitCnt )
{
	m_bInit = FALSE;

	BITMAPINFO *pbmiDIB = ( BITMAPINFO * ) &m_dibheader;

	memset( &m_dibheader, 0, sizeof( DIBinfo ) );

	if ( nBitCnt == 0 )
		nBitCnt = 8;
	if ( m_hDC == NULL )
	{
		return FALSE;
	}

	BYTE* pBuffer = NULL;

	// figure out if we're running in an 8-bit display mode
	if ( ::GetDeviceCaps( m_hDC, RASTERCAPS ) & RC_PALETTE )
	{
		m_bPalettized = TRUE;

		// save system colors
		if ( !m_bSystemcolorsSaved )
		{
			SaveSystemColors();
			m_bSystemcolorsSaved = TRUE;
		}
	}
	else
	{
		m_bPalettized = FALSE;
	}

	if ( m_bPalettized == TRUE )
	{
		::MessageBox( NULL, _T("Error: Do NOT use 8-bit display mode"), _T("Error"), MB_OK|MB_ICONSTOP );
		return FALSE;
	}

	// fill in the BITMAPINFO struct
	pbmiDIB->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	pbmiDIB->bmiHeader.biWidth         = m_nWidth;
	pbmiDIB->bmiHeader.biHeight        = m_nHeight;
	pbmiDIB->bmiHeader.biPlanes        = 1;
	pbmiDIB->bmiHeader.biBitCount      = nBitCnt;
	pbmiDIB->bmiHeader.biCompression   = BI_RGB;
	pbmiDIB->bmiHeader.biSizeImage     = 0;
	pbmiDIB->bmiHeader.biXPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biYPelsPerMeter = 0;
	pbmiDIB->bmiHeader.biClrUsed       = 256;
	pbmiDIB->bmiHeader.biClrImportant  = 256;

	m_nBitCnt = nBitCnt;

	// fill in the palette
	if ( nBitCnt == 8 )
	{
		int i;
		if ( m_bSetPalette )
		{
			BYTE* pPalPtr = m_pPalette;
			for ( i = 0; i < 256; i++, pPalPtr+=4 )
			{
				m_dibheader.acolors[i].rgbRed   = pPalPtr[0];//( m_pn8to24table[i] >> 0 )  & 0xff;
				m_dibheader.acolors[i].rgbGreen = pPalPtr[1];//( m_pn8to24table[i] >> 8 )  & 0xff;
				m_dibheader.acolors[i].rgbBlue  = pPalPtr[2];//( m_pn8to24table[i] >> 16 ) & 0xff;
			}
		}
		else
		{
			for ( i = 0; i < 256; i++ )
			{
				m_dibheader.acolors[i].rgbRed   = i;//( m_pn8to24table[i] >> 0 )  & 0xff;
				m_dibheader.acolors[i].rgbGreen = i;//( m_pn8to24table[i] >> 8 )  & 0xff;
				m_dibheader.acolors[i].rgbBlue  = i;//( m_pn8to24table[i] >> 16 ) & 0xff;
			}
		}
	}

	::GdiFlush();

	// create the DIB section
	m_hDIBSection = ::CreateDIBSection(	m_hDC,
										pbmiDIB,
										DIB_RGB_COLORS,
										(VOID **)&m_pDIBBase,
										NULL,
										0 );

	int nWidthBytes;
	DWORD Err = ::GetLastError();	// TODO: free error
	if ( m_hDIBSection == NULL )
	{
		TRACE1( "CxDirectDIB::Init(), %d - CreateDIBSection failed\r\n", ::GetLastError() );
		goto fail;
	}

	int nPitch;

	if ( pbmiDIB->bmiHeader.biHeight > 0 )
    {
		// bottom up
		pBuffer	= m_pDIBBase + ( m_nHeight - 1 ) * GetWidthBytes( m_nWidth, nBitCnt );;
		nPitch	= -GetWidthBytes( m_nWidth, nBitCnt );
    }
    else
    {
		// top down
		pBuffer	= m_pDIBBase;
		nPitch	= GetWidthBytes( m_nWidth, nBitCnt );
    }

	m_pOffBuffer = pBuffer;
	m_nPitch = nPitch;

	nWidthBytes = m_nPitch < 0 ? -m_nPitch : m_nPitch;

	// clear the DIB memory buffer
	//memset( m_pDIBBase, 0x00, nWidthBytes * m_nHeight );

	if ( ( m_hdcDIBSection = ::CreateCompatibleDC( m_hDC ) ) == NULL )
	{
		TRACE0( "CxDirectDIB::Init() - CreateCompatibleDC failed\r\n" );
		goto fail;
	}

	if ( ( m_hPreviouslySelectedGDIObj = ::SelectObject( m_hdcDIBSection, m_hDIBSection ) ) == NULL )
	{
		TRACE0( "CxDirectDIB::Init() - SelectObject failed\r\n" );
		goto fail;
	}

	m_hDrawDib = ::DrawDibOpen();
	::DrawDibBegin( m_hDrawDib, m_hDC, m_nWidth, m_nHeight, &pbmiDIB->bmiHeader, m_nWidth, m_nHeight, DDF_SAME_HDC|DDF_SAME_DRAW);

	ASSERT( m_hDrawDib );

	m_bInit = TRUE;
	return TRUE;

fail:
	m_bInit = FALSE;
	Shutdown();
	return FALSE;
}

void CxDirectDIB::SetPalette( const BYTE * pPal )
{
	BYTE* pPalPtr = (BYTE*)pPal;
	for ( int i = 0; i < 256; i++, pPalPtr+=4 )
	{
		m_dibheader.acolors[i].rgbRed   = pPalPtr[0];
		m_dibheader.acolors[i].rgbGreen = pPalPtr[1];
		m_dibheader.acolors[i].rgbBlue  = pPalPtr[2];
	}

	memcpy( m_pPalette, pPal, sizeof(BYTE)*256*4 );

	m_bSetPalette = TRUE;

	return;
}

void CxDirectDIB::Shutdown()
{
	if ( !m_hDC ) return;

	if ( m_bPalettized && m_bSystemcolorsSaved )
		RestoreSystemColors();

	if ( m_hPal )
	{
		::DeleteObject( m_hPal );
		m_hPal = NULL;
	}

	if ( m_hOldPal )
	{
		::SelectPalette( m_hDC, m_hOldPal, FALSE );
		::RealizePalette( m_hDC );
		m_hOldPal = NULL;
	}

	if ( m_hdcDIBSection )
	{
		::SelectObject( m_hdcDIBSection, m_hPreviouslySelectedGDIObj );
		::DeleteDC( m_hdcDIBSection );
		m_hdcDIBSection = NULL;
	}

	if ( m_hDIBSection )
	{
		::DeleteObject( m_hDIBSection );
		m_hDIBSection = NULL;
		m_pDIBBase = NULL;
	}

	if ( m_hDrawDib != NULL )
	{
		::DrawDibEnd( m_hDrawDib );
		::DrawDibClose( m_hDrawDib );
		m_hDrawDib = NULL;
	}

	m_hDC = NULL;
}

BOOL CxDirectDIB::EndDraw()
{
	ASSERT( m_hDC != NULL );
	if ( !m_hDC ) return FALSE;
	if ( !m_hDrawDib ) return FALSE;

	BITMAPINFO *pbmiDIB = ( BITMAPINFO * ) &m_dibheader;

//	::DrawDibBegin( m_hDrawDib, m_hDC, m_nWidth, m_nHeight, &pbmiDIB->bmiHeader, m_nWidth, m_nHeight, DDF_SAME_HDC|DDF_SAME_DRAW);

#if 0
	::BitBlt(	m_hDC,
				0, 0,
				m_nWidth,
				m_nHeight,
				m_hdcDIBSection,
				0, 0,
				SRCCOPY );
#else

	::DrawDibDraw( m_hDrawDib, m_hDC, 0, 0, m_nWidth, m_nHeight, 
			&pbmiDIB->bmiHeader, m_pDIBBase, 0, 0, m_nWidth, m_nHeight, DDF_SAME_HDC|DDF_SAME_DRAW );
#endif

//	::DrawDibEnd( m_hDrawDib );

	return TRUE;
}

BOOL CxDirectDIB::EndDraw( RECT rectRedraw )
{
	ASSERT( m_hDC != NULL );
	if ( !m_hDC ) return FALSE;
	ASSERT( m_hDrawDib != NULL );
	
	BITMAPINFO *pbmiDIB = ( BITMAPINFO * ) &m_dibheader;

	::DrawDibDraw( m_hDrawDib, m_hDC, 
			rectRedraw.left, rectRedraw.top, 
			rectRedraw.right-rectRedraw.left, rectRedraw.bottom-rectRedraw.top, 
			&pbmiDIB->bmiHeader, m_pDIBBase, 
			rectRedraw.left, rectRedraw.top, 
			rectRedraw.right-rectRedraw.left, rectRedraw.bottom-rectRedraw.top, 
			DDF_SAME_HDC|DDF_SAME_DRAW );
	return TRUE;
}

const BYTE* CxDirectDIB::GetPalette()
{
	return m_pPalette;
}