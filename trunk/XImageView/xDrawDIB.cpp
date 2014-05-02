// xDrawDIB.cpp: implementation of the CxDrawDIB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "xDrawDIB.h"

#include <XImage/xImageObject.h>
#include <XUtil/xCriticalSection.h>
#include "xRender2D.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CxDrawDIB::CxDrawDIB( CxRender2D* pRenderer ) : m_pRenderer(pRenderer)
{

}

CxDrawDIB::CxDrawDIB() : m_pRenderer(NULL)
{

}

CxDrawDIB::~CxDrawDIB()
{
	m_pRenderer = NULL;
}

void CxDrawDIB::SetDevice( CxRender2D* pRenderer )
{
	m_pRenderer = pRenderer;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL CxDrawDIB::Draw( CxImageObject* pImgObj, 
						 int nDstX, int nDstY, int nDstW, int nDstH, 
						 int nSrcX, int nSrcY, int nSrcW, int nSrcH, 
						 float fZoomRatio )
{
	ASSERT( m_pRenderer != NULL );
	ASSERT( nSrcW != 0 && nSrcH != 0 );

//	if ( pImgObj->GetBpp() != m_pRenderer->GetBufferBitsCnt() )
//		return FALSE;
	
	BYTE nAlpha = 0;

	int nWidth, nHeight;
	m_pRenderer->GetDims( nWidth, nHeight );
	//TRACE( _T("Renderer Dimension(%dx%d)\r\n"), nWidth, nHeight );
	int nWidthBytes;
	
	m_pRenderer->Lock();
	BYTE* pBuf = m_pRenderer->GetOffBuffer( nWidthBytes );
	if ( !pBuf )
		return FALSE;

	//int nRatioX = 1000.f/fZoomRatio;//nSrcW * 1000 / nDstW;
	//int nRatioY = 1000.f/fZoomRatio;//nSrcH * 1000 / nDstH;
	float fRatioX = 1.f/fZoomRatio;//(float)nSrcW / nDstW;
	float fRatioY = 1.f/fZoomRatio;//(float)nSrcH / nDstH;


	nDstY < 0 ? nDstY = 0 : nDstY;
	nDstX < 0 ? nDstX = 0 : nDstX;
	int nDstOffW = /*nDstX+*/nDstW;
	int nDstOffH = /*nDstY+*/nDstH;
	nDstOffW+nDstX > nWidth ? nDstOffW = nWidth-nDstX : nDstOffW;
	nDstOffH+nDstY > nHeight ? nDstOffH = nHeight-nDstY : nDstOffH;

	CxCriticalSection::Owner Lock( *pImgObj->GetImageLockObject() );

	nSrcW = (nSrcW+nSrcX) >= pImgObj->GetWidth() ? pImgObj->GetWidth() - nSrcX : nSrcW;
	nSrcH = (nSrcH+nSrcY) >= pImgObj->GetHeight() ? pImgObj->GetHeight() - nSrcY : nSrcH;

	if ( m_pRenderer->GetBufferBitsCnt() == 32 )
	{
		if ( fZoomRatio != 1.f )
		{
			if (pImgObj->GetChannelSeq() == CxImageObject::ChannelSeqRGB)
				Screen32ScaledBlitRGB( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH, fRatioX, fRatioY );
			else
				Screen32ScaledBlitBGR( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH, fRatioX, fRatioY );
		}
		else
		{
			if (pImgObj->GetChannelSeq() == CxImageObject::ChannelSeqRGB)
				Screen32BlitRGB( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH );
			else
				Screen32BlitBGR( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH );

		}
	}
	else
	{
		if ( (fZoomRatio != 1.f) || (pImgObj->GetChannel() == 1 && pImgObj->GetDepth() == 16) || (pImgObj->GetChannelSeq() == CxImageObject::ChannelSeqRGB))
		{
			ScreenScaledBlit( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH, fRatioX, fRatioY );
		}
		else
		{
			ScreenBlit( pBuf, nWidthBytes, pImgObj, nDstX, nDstY, nDstOffW, nDstOffH, nSrcX, nSrcY, nSrcW, nSrcH );
		}
	}

	m_pRenderer->Unlock();

	return TRUE;
}

void CxDrawDIB::ScreenBlit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							int nDstOffsetY/*=0*/,
							int nDstOffsetH/*=0*/ )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	int i;
	size_t nIndex;

	#define OFF_BUF( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + (x) ))

	if ( nDstOffH > nSrcH ) nDstOffH = nSrcH;
	if ( nDstOffW > nSrcW ) nDstOffW = nSrcW;

	// 2009/06/29
	nDstOffH += nDstY;
	nDstOffW += nDstX;

	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;

			memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX, pImgBuf+nSrcX+nTH*nSrcWidthBytes, nSrcW );
		}
		break;
	case 24:
		nIndex = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;

			memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX*3, pImgBuf+nSrcX*3+nTH*nSrcWidthBytes, nSrcW*3 );
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF
}

void CxDrawDIB::Screen32BlitBGR( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							int nDstOffsetY/*=0*/,
							int nDstOffsetH/*=0*/ )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	size_t nIndex;

	size_t nIndex1;
	size_t nIndex2;
	size_t nIndex3;
	#define OFF_BUF_B( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+0 ))
	#define OFF_BUF_G( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+1 ))
	#define OFF_BUF_R( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+2 ))

	if ( nDstOffH > nSrcH ) nDstOffH = nSrcH;
	if ( nDstOffW > nSrcW ) nDstOffW = nSrcW;

	// 2009/06/29
	nDstOffW += nDstX;
	nDstOffH += nDstY;

	const BYTE* pPalette = m_pRenderer->GetPalette();

	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( int i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			for ( int j=nDstX ; j < (nDstOffW & ~7) ; j+=8 ) 
			{ 
				size_t nTW0 = size_t(j-nDstX)+nSrcX;
				if ( nTW0 >= (size_t)nImgWidth || nTW0 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW0;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW1 = size_t(j+1-nDstX)+nSrcX;
				if ( nTW1 >= (size_t)nImgWidth || nTW1 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW1;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW2 = size_t(j+2-nDstX)+nSrcX;
				if ( nTW2 >= (size_t)nImgWidth || nTW2 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW2;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW3 = size_t(j+3-nDstX)+nSrcX;
				if ( nTW3 >= (size_t)nImgWidth || nTW3 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW3;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW4 = size_t(j+4-nDstX)+nSrcX;
				if ( nTW4 >= (size_t)nImgWidth || nTW4 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW4;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW5 = size_t(j+5-nDstX)+nSrcX;
				if ( nTW5 >= (size_t)nImgWidth || nTW5 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW5;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW6 = size_t(j+6-nDstX)+nSrcX;
				if ( nTW6 >= (size_t)nImgWidth || nTW6 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW6;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW7 = size_t(j+7-nDstX)+nSrcX;
				if ( nTW7 >= (size_t)nImgWidth || nTW7 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW7;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}

			for ( int j=(nDstOffW & ~7) ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t(j-nDstX)+nSrcX;
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex = nTH * nSrcWidthBytes + nTW;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}
			//memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX, pImgBuf+nSrcX+nTH*nSrcWidthBytes, nSrcW );

		}
		break;
	case 24:
		nIndex1 = 0;
		nIndex2 = 0;
		nIndex3 = 0;
		for ( int i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;

			//memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX*3, pImgBuf+nSrcX*3+nTH*nSrcWidthBytes, nSrcW*3 );
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF_B
	#undef OFF_BUF_G
	#undef OFF_BUF_R
}

void CxDrawDIB::Screen32BlitRGB( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							int nDstOffsetY/*=0*/,
							int nDstOffsetH/*=0*/ )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	size_t nIndex;

	size_t nIndex1;
	size_t nIndex2;
	size_t nIndex3;
	#define OFF_BUF_B( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+2 ))
	#define OFF_BUF_G( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+1 ))
	#define OFF_BUF_R( x, y ) (*( pScreenBuffer + (y * nScrnWidthBytes) + ((x)<<2)+0 ))

	if ( nDstOffH > nSrcH ) nDstOffH = nSrcH;
	if ( nDstOffW > nSrcW ) nDstOffW = nSrcW;

	// 2009/06/29
	nDstOffW += nDstX;
	nDstOffH += nDstY;

	const BYTE* pPalette = m_pRenderer->GetPalette();

	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( int i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			for ( int j=nDstX ; j < (nDstOffW & ~7) ; j+=8 ) 
			{ 
				size_t nTW0 = size_t(j-nDstX)+nSrcX;
				if ( nTW0 >= (size_t)nImgWidth || nTW0 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW0;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW1 = size_t(j+1-nDstX)+nSrcX;
				if ( nTW1 >= (size_t)nImgWidth || nTW1 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW1;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW2 = size_t(j+2-nDstX)+nSrcX;
				if ( nTW2 >= (size_t)nImgWidth || nTW2 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW2;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW3 = size_t(j+3-nDstX)+nSrcX;
				if ( nTW3 >= (size_t)nImgWidth || nTW3 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW3;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW4 = size_t(j+4-nDstX)+nSrcX;
				if ( nTW4 >= (size_t)nImgWidth || nTW4 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW4;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW5 = size_t(j+5-nDstX)+nSrcX;
				if ( nTW5 >= (size_t)nImgWidth || nTW5 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW5;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW6 = size_t(j+6-nDstX)+nSrcX;
				if ( nTW6 >= (size_t)nImgWidth || nTW6 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW6;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
				size_t nTW7 = size_t(j+7-nDstX)+nSrcX;
				if ( nTW7 >= (size_t)nImgWidth || nTW7 < 0 ) continue;
				nIndex = nTH * nSrcWidthBytes + nTW7;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}

			for ( int j=(nDstOffW & ~7) ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t(j-nDstX)+nSrcX;
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex = nTH * nSrcWidthBytes + nTW;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}
			//memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX, pImgBuf+nSrcX+nTH*nSrcWidthBytes, nSrcW );

		}
		break;
	case 24:
		nIndex1 = 0;
		nIndex2 = 0;
		nIndex3 = 0;
		for ( int i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t(i-nDstY)+nSrcY;

			//memcpy( pScreenBuffer+i*nScrnWidthBytes+nDstX*3, pImgBuf+nSrcX*3+nTH*nSrcWidthBytes, nSrcW*3 );
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF_B
	#undef OFF_BUF_G
	#undef OFF_BUF_R
}


void CxDrawDIB::ScreenScaledBlit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
								 int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
								 int nSrcX, int nSrcY, int nSrcW, int nSrcH,
								 float fRatioX, float fRatioY,
								 int nDstOffsetY/*=0*/,
								 int nDstOffsetH/*=0*/ )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	int i, j;
	size_t nIndex;

	int nPixelMax = pImgObj->GetPixelMaximum();

	size_t nIndex1;
	size_t nIndex2;
	size_t nIndex3;
	#define OFF_BUF( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + (x) ))

	// 2009/06/29
	nDstOffW += nDstX;
	nDstOffH += nDstY;

	size_t nTHSrcWidthBytes;
	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			nTHSrcWidthBytes = nTH*nSrcWidthBytes;

			for ( j=nDstX ; j < ((nDstOffW) & ~7) ; j+=8 ) 
			{ 
				size_t nTW0 = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW0 >= (size_t)nImgWidth || nTW0 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW0;
				OFF_BUF( j, i ) = pImgBuf[ nIndex ];

				size_t nTW1 = size_t((j+1-nDstX)*fRatioX+nSrcX);
				if ( nTW1 >= (size_t)nImgWidth || nTW1 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW1;
				OFF_BUF( j+1, i ) = pImgBuf[ nIndex ];

				size_t nTW2 = size_t((j+2-nDstX)*fRatioX+nSrcX);
				if ( nTW2 >= (size_t)nImgWidth || nTW2 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW2;
				OFF_BUF( j+2, i ) = pImgBuf[ nIndex ];

				size_t nTW3 = size_t((j+3-nDstX)*fRatioX+nSrcX);
				if ( nTW3 >= (size_t)nImgWidth || nTW3 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW3;
				OFF_BUF( j+3, i ) = pImgBuf[ nIndex ];

				size_t nTW4 = size_t((j+4-nDstX)*fRatioX+nSrcX);
				if ( nTW4 >= (size_t)nImgWidth || nTW4 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW4;
				OFF_BUF( j+4, i ) = pImgBuf[ nIndex ];

				size_t nTW5 = size_t((j+5-nDstX)*fRatioX+nSrcX);
				if ( nTW5 >= (size_t)nImgWidth || nTW5 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW5;
				OFF_BUF( j+5, i ) = pImgBuf[ nIndex ];

				size_t nTW6 = size_t((j+6-nDstX)*fRatioX+nSrcX);
				if ( nTW6 >= (size_t)nImgWidth || nTW6 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW6;
				OFF_BUF( j+6, i ) = pImgBuf[ nIndex ];

				size_t nTW7 = size_t((j+7-nDstX)*fRatioX+nSrcX);
				if ( nTW7 >= (size_t)nImgWidth || nTW7 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW7;
				OFF_BUF( j+7, i ) = pImgBuf[ nIndex ];
			}

			for ( j=((nDstOffW) & ~7) ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTH >= (size_t)nImgHeight || nTW >= (size_t)nImgWidth || nTW < 0 || nTH < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW;
				OFF_BUF( j, i ) = pImgBuf[ nIndex ];
			}
		}
		break;
	case 16:
		if (pImgObj->GetChannel() != 1)
			break;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;
			nTHSrcWidthBytes = nTH*nSrcWidthBytes;
			for ( j=nDstX ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW*2;
				OFF_BUF( j, i ) = (*(unsigned short*)(pImgBuf + nIndex)) * 255 / nPixelMax;
			}
		}
		break;
	case 24:
		nIndex1 = 0;
		nIndex2 = 0;
		nIndex3 = 0;
		if (pImgObj->GetChannelSeq() == CxImageObject::ChannelSeqRGB)
		{
			for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
			{
				size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
				if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;
				nTHSrcWidthBytes = nTH*nSrcWidthBytes;
				for ( j=nDstX ; j < nDstOffW ; j++ ) 
				{ 
					size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);

					if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

					nIndex1 = nTHSrcWidthBytes + nTW*3+0;
					nIndex2 = nTHSrcWidthBytes + nTW*3+1;
					nIndex3 = nTHSrcWidthBytes + nTW*3+2;

					OFF_BUF( j*3+2, i ) = pImgBuf[ nIndex1 ];
					OFF_BUF( j*3+1, i ) = pImgBuf[ nIndex2 ];
					OFF_BUF( j*3+0, i ) = pImgBuf[ nIndex3 ];
				}
			}
		}
		else
		{
			for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
			{
				size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
				if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;
				nTHSrcWidthBytes = nTH*nSrcWidthBytes;
				for ( j=nDstX ; j < nDstOffW ; j++ ) 
				{ 
					size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);

					if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

					nIndex1 = nTHSrcWidthBytes + nTW*3+0;
					nIndex2 = nTHSrcWidthBytes + nTW*3+1;
					nIndex3 = nTHSrcWidthBytes + nTW*3+2;

					OFF_BUF( j*3+0, i ) = pImgBuf[ nIndex1 ];
					OFF_BUF( j*3+1, i ) = pImgBuf[ nIndex2 ];
					OFF_BUF( j*3+2, i ) = pImgBuf[ nIndex3 ];
				}
			}
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF
}

void CxDrawDIB::Screen32ScaledBlitBGR( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
								 int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
								 int nSrcX, int nSrcY, int nSrcW, int nSrcH,
								 float fRatioX, float fRatioY,
								 int nDstOffsetY/*=0*/,
								 int nDstOffsetH/*=0*/  )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	int i, j;
	size_t nIndex;

	size_t nIndex1;
	size_t nIndex2;
	size_t nIndex3;
	#define OFF_BUF_B( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+0 ))
	#define OFF_BUF_G( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+1 ))
	#define OFF_BUF_R( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+2 ))

	const BYTE* pPalette = m_pRenderer->GetPalette();

	// 2009/06/29
	nDstOffW += nDstX;
	nDstOffH += nDstY;
	
	size_t nTHSrcWidthBytes;
	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			nTHSrcWidthBytes = nTH*nSrcWidthBytes;
			for ( j=nDstX ; j < (nDstOffW & ~7) ; j+=8 ) 
			{ 
				size_t nTW0 = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW0 >= (size_t)nImgWidth || nTW0 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW0;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW1 = size_t((j+1-nDstX)*fRatioX+nSrcX);
				if ( nTW1 >= (size_t)nImgWidth || nTW1 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW1;
				OFF_BUF_B( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW2 = size_t((j+2-nDstX)*fRatioX+nSrcX);
				if ( nTW2 >= (size_t)nImgWidth || nTW2 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW2;
				OFF_BUF_B( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW3 = size_t((j+3-nDstX)*fRatioX+nSrcX);
				if ( nTW3 >= (size_t)nImgWidth || nTW3 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW3;
				OFF_BUF_B( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW4 = size_t((j+4-nDstX)*fRatioX+nSrcX);
				if ( nTW4 >= (size_t)nImgWidth || nTW4 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW4;
				OFF_BUF_B( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW5 = size_t((j+5-nDstX)*fRatioX+nSrcX);
				if ( nTW5 >= (size_t)nImgWidth || nTW5 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW5;
				OFF_BUF_B( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW6 = size_t((j+6-nDstX)*fRatioX+nSrcX);
				if ( nTW6 >= (size_t)nImgWidth || nTW6 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW6;
				OFF_BUF_B( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW7 = size_t((j+7-nDstX)*fRatioX+nSrcX);
				if ( nTW7 >= (size_t)nImgWidth || nTW7 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW7;
				OFF_BUF_B( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}

			for ( j=(nDstOffW & ~7) ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}
		}
		break;
	case 24:
		nIndex1 = 0;
		nIndex2 = 0;
		nIndex3 = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			nTHSrcWidthBytes = nTH * nSrcWidthBytes;
			for ( j=nDstX ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex1 = nTHSrcWidthBytes + nTW*3+0;
				nIndex2 = nTHSrcWidthBytes + nTW*3+1;
				nIndex3 = nTHSrcWidthBytes + nTW*3+2;

				OFF_BUF_B( j, i ) = pImgBuf[ nIndex1 ];
				OFF_BUF_G( j, i ) = pImgBuf[ nIndex2 ];
				OFF_BUF_R( j, i ) = pImgBuf[ nIndex3 ];
			}
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF_B
	#undef OFF_BUF_G
	#undef OFF_BUF_R
}

void CxDrawDIB::Screen32ScaledBlitRGB( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
								 int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
								 int nSrcX, int nSrcY, int nSrcW, int nSrcH,
								 float fRatioX, float fRatioY,
								 int nDstOffsetY/*=0*/,
								 int nDstOffsetH/*=0*/  )
{
	int nSrcWidthBytes = pImgObj->GetWidthBytes();

	BYTE* pImgBuf = (BYTE*)pImgObj->GetImageBuffer();
	
	int nImgHeight = pImgObj->GetHeight();
	int nImgWidth = pImgObj->GetWidth();

	int i, j;
	size_t nIndex;

	size_t nIndex1;
	size_t nIndex2;
	size_t nIndex3;
	#define OFF_BUF_B( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+2 ))
	#define OFF_BUF_G( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+1 ))
	#define OFF_BUF_R( x, y ) (*( pScreenBuffer + (size_t(y) * nScrnWidthBytes) + ((x)<<2)+0 ))

	const BYTE* pPalette = m_pRenderer->GetPalette();

	// 2009/06/29
	nDstOffW += nDstX;
	nDstOffH += nDstY;
	
	size_t nTHSrcWidthBytes;
	switch ( pImgObj->GetBpp() )
	{
	case 8:
		nIndex = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			nTHSrcWidthBytes = nTH*nSrcWidthBytes;
			for ( j=nDstX ; j < (nDstOffW & ~7) ; j+=8 ) 
			{ 
				size_t nTW0 = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW0 >= (size_t)nImgWidth || nTW0 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW0;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW1 = size_t((j+1-nDstX)*fRatioX+nSrcX);
				if ( nTW1 >= (size_t)nImgWidth || nTW1 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW1;
				OFF_BUF_B( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+1, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW2 = size_t((j+2-nDstX)*fRatioX+nSrcX);
				if ( nTW2 >= (size_t)nImgWidth || nTW2 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW2;
				OFF_BUF_B( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+2, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW3 = size_t((j+3-nDstX)*fRatioX+nSrcX);
				if ( nTW3 >= (size_t)nImgWidth || nTW3 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW3;
				OFF_BUF_B( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+3, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW4 = size_t((j+4-nDstX)*fRatioX+nSrcX);
				if ( nTW4 >= (size_t)nImgWidth || nTW4 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW4;
				OFF_BUF_B( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+4, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW5 = size_t((j+5-nDstX)*fRatioX+nSrcX);
				if ( nTW5 >= (size_t)nImgWidth || nTW5 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW5;
				OFF_BUF_B( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+5, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW6 = size_t((j+6-nDstX)*fRatioX+nSrcX);
				if ( nTW6 >= (size_t)nImgWidth || nTW6 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW6;
				OFF_BUF_B( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+6, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];

				size_t nTW7 = size_t((j+7-nDstX)*fRatioX+nSrcX);
				if ( nTW7 >= (size_t)nImgWidth || nTW7 < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW7;
				OFF_BUF_B( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j+7, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}

			for ( j=(nDstOffW & ~7) ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex = nTHSrcWidthBytes + nTW;
				OFF_BUF_B( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 2];
				OFF_BUF_G( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 1];
				OFF_BUF_R( j, i ) = pPalette[ (pImgBuf[ nIndex ] << 2) + 0];
			}
		}
		break;
	case 24:
		nIndex1 = 0;
		nIndex2 = 0;
		nIndex3 = 0;
		for ( i=nDstY+nDstOffsetY ; i < nDstOffH-nDstOffsetH ; i++ )
		{
			size_t nTH = size_t((i-nDstY)*fRatioY+nSrcY);
			if ( nTH >= (size_t)nImgHeight || nTH < 0 ) continue;

			nTHSrcWidthBytes = nTH * nSrcWidthBytes;
			for ( j=nDstX ; j < nDstOffW ; j++ ) 
			{ 
				size_t nTW = size_t((j-nDstX)*fRatioX+nSrcX);
				if ( nTW >= (size_t)nImgWidth || nTW < 0 ) continue;

				nIndex1 = nTHSrcWidthBytes + nTW*3+0;
				nIndex2 = nTHSrcWidthBytes + nTW*3+1;
				nIndex3 = nTHSrcWidthBytes + nTW*3+2;

				OFF_BUF_B( j, i ) = pImgBuf[ nIndex1 ];
				OFF_BUF_G( j, i ) = pImgBuf[ nIndex2 ];
				OFF_BUF_R( j, i ) = pImgBuf[ nIndex3 ];
			}
		}
		break;
	default:
		ASSERT( FALSE );	// not supported bits per pixel
		break;		
	}
	
	#undef OFF_BUF_B
	#undef OFF_BUF_G
	#undef OFF_BUF_R
}