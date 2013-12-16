#include "stdafx.h"
#include <XImage/xImageObject.h>

#include <XUtil/xCriticalSection.h>
#include <XUtil/DebugSupport/xDebug.h>

#pragma warning(disable: 4819)
#include <opencv/cv.h>
#include <opencv/HighGUI.h>

#include <XUtil/String/xString.h>

#include <atlconv.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxImageObject::CxImageObject() :
	m_pIPLImage(NULL), m_bDelete(FALSE), 
	m_fnOnImageProgress(NULL), m_bNotifyChangeImage(FALSE),
	m_hBitmap(NULL)
{
	m_pCsLockImage = new CxCriticalSection();
}

CxImageObject::~CxImageObject()
{
	Destroy();

	if ( m_pCsLockImage )
	{
		delete m_pCsLockImage;
		m_pCsLockImage = NULL;
	}
}

void CxImageObject::SetData1( DWORD_PTR dwUsrData1 )
{
	m_dwUsrData1 = dwUsrData1;
}

DWORD_PTR CxImageObject::GetData1()
{ 
	return m_dwUsrData1; 
}

void CxImageObject::SetData2( DWORD_PTR dwUsrData2 )
{ 
	m_dwUsrData2 = dwUsrData2; 
}

DWORD_PTR CxImageObject::GetData2()
{ 
	return m_dwUsrData2; 
}

void CxImageObject::SetData3( DWORD_PTR dwUsrData3 )
{
	m_dwUsrData3 = dwUsrData3;
}

DWORD_PTR CxImageObject::GetData3()
{
	return m_dwUsrData3;
}

BOOL CxImageObject::IsValid() const
{
	return m_pIPLImage!=NULL ? TRUE : FALSE;
}

int CxImageObject::GetWidthBytes() const
{
	return !m_pIPLImage ? 0 : m_pIPLImage->widthStep;
}

int CxImageObject::GetWidthBytes( int nCx, int nBitCount )
{
	DWORD dwBytes = nCx * nBitCount;
	return ((dwBytes & 0x001f) ? (dwBytes >> 5) + 1 : dwBytes >> 5) << 2;
}

LPVOID CxImageObject::GetImageBuffer() const
{
	return !m_pIPLImage ? NULL : m_pIPLImage->imageData;
}

int CxImageObject::GetWidth() const 
{ 
	return !m_pIPLImage ? 0 : m_pIPLImage->width; 
}

int CxImageObject::GetHeight() const
{ 
	return !m_pIPLImage ? 0 : m_pIPLImage->height; 
}

int CxImageObject::GetBpp() const
{
	return m_pIPLImage ? (m_pIPLImage->depth & 255)*m_pIPLImage->nChannels : 0;
}

CxCriticalSection*	CxImageObject::GetImageLockObject()
{
	return m_pCsLockImage;
}

struct _IplImage* CxImageObject::GetImage() const
{
	return m_pIPLImage;
}

void CxImageObject::_OnProgress( int nProgress, LPVOID lpUsrData )
{
	XTRACE( _T("Progress: %d\r\n"), nProgress );
	CxImageObject* pThis = (CxImageObject*)lpUsrData;
	pThis->OnProgress( nProgress );
	if ( pThis->m_fnOnImageProgress )
	{
		(*pThis->m_fnOnImageProgress)( nProgress );
	}
}

BOOL CxImageObject::IsHBitmapAttached()
{
	return m_hBitmap != NULL ? TRUE : FALSE;
}

void CxImageObject::AttachHBitmap( HBITMAP hBitmap )
{
	XASSERT( m_hBitmap == NULL );

	BITMAP bm;
	if ( !::GetObject( hBitmap, sizeof(BITMAP), (LPSTR)&bm ) ) return;
	
	WORD cClrBits = (WORD)(bm.bmPlanes * bm.bmBitsPixel); 
    if (cClrBits == 1)			// unsupport
        return;
    else if (cClrBits <= 4)		// unsupport
        return;
    else if (cClrBits <= 8) 
        cClrBits = 8; 
    else if (cClrBits <= 16) 
        cClrBits = 16; 
    else if (cClrBits <= 24) 
        cClrBits = 24; 
    else cClrBits = 32;
	
	int nSrcBytes = sizeof(unsigned char) * bm.bmWidthBytes * bm.bmHeight;

	BYTE* pSrcBuffer = new BYTE[nSrcBytes];

	LONG lReadBytes = ::GetBitmapBits( hBitmap, nSrcBytes, pSrcBuffer );

	Destroy();

	int nDestW = bm.bmHeight;
	int nDestH = bm.bmWidth;

	if ( cClrBits == 8 )
		Create( nDestW, nDestH, 8, 0 );
	else
		Create( nDestW, nDestH, 24, 0 );

	int nDestWBytes = GetWidthBytes();

	switch ( cClrBits )
	{
	case 8:
		XASSERT( nSrcBytes == GetWidthBytes() * GetHeight() );
		memcpy( (BYTE*)m_pIPLImage->imageData, pSrcBuffer, nSrcBytes );
		break;
	case 16:
		for ( int i=0 ; i<nDestH ; i++ )
		{
			for ( int j=0 ; j<nDestW ; j++ )
			{
				BYTE* pDest = (BYTE*)m_pIPLImage->imageData+nDestWBytes*i+j*3;
				BYTE* pSrc = pSrcBuffer+bm.bmWidthBytes*i+j*2;
				// 5 6 5
				// rrrr rggg gggb bbbb
				*(pDest+2) = (pSrc[1] >> 3) << 3 | 0x07;							// R
				*(pDest+1) = ((pSrc[1] & 0x07) << 3 | (pSrc[0] >> 5)) << 2 | 0x03;	// G
				*(pDest+0) = (pSrc[0] & 0x1F) << 3 | 0x07;							// B
			}
		}
		break;
	case 24:
		for ( int i=0 ; i<nDestH ; i++ )
		{
			BYTE* pDest = (BYTE*)m_pIPLImage->imageData+nDestWBytes*i;
			BYTE* pSrc = (BYTE*)pSrcBuffer+bm.bmWidthBytes*i;
			memcpy( pDest, pSrc, bm.bmWidth );
		}
		break;
	case 32:
		for ( int i=0 ; i<nDestH ; i++ )
		{
			for ( int j=0 ; j<nDestW ; j++ )
			{
				BYTE* pDest = (BYTE*)m_pIPLImage->imageData+nDestWBytes*i+j*3;
				BYTE* pSrc = pSrcBuffer+bm.bmWidthBytes*i+j*4;
				*(pDest+0) = *(pSrc+0); // B
				*(pDest+1) = *(pSrc+1); // G
				*(pDest+2) = *(pSrc+2); // R
			}
		}
		break;
	}

	delete[] pSrcBuffer;

	m_hBitmap = hBitmap;

}

void CxImageObject::DetachHBitmap()
{
	XASSERT( m_hBitmap != NULL );
	Destroy();

	m_hBitmap = NULL;
}

void CxImageObject::ClearNotifyFlag()
{
	m_bNotifyChangeImage = FALSE;
}

BOOL CxImageObject::IsNotifyFlag()
{
	return m_bNotifyChangeImage;
}

BOOL CxImageObject::LoadFromFileA( LPCSTR lpszFileName, BOOL bForceGray8/*=FALSE*/ )
{
	CxCriticalSection::Owner Lock(*m_pCsLockImage);
	if ( !m_bDelete )
		Destroy();

	try
	{
		m_pIPLImage = cvLoadImage( lpszFileName, bForceGray8 ? CV_LOAD_IMAGE_GRAYSCALE : CV_LOAD_IMAGE_UNCHANGED );	// 2nd parameter: 0-gray, 1-color
	}
	catch (cv::Exception& e)
	{
		CxString strError = e.what();
		XTRACE( _T("CxImageObject::LoadFromFile Error - %s\n"), strError );
		return FALSE;
	}

	m_bDelete = TRUE;
	m_bNotifyChangeImage = TRUE;

	return m_pIPLImage ? TRUE : FALSE;
}

BOOL CxImageObject::LoadFromFileW( LPCWSTR lpszFileName, BOOL bForceGray8/*=FALSE*/ )
{
	USES_CONVERSION;
	return LoadFromFileA( W2A((LPWSTR)lpszFileName), bForceGray8 );
}

BOOL CxImageObject::SaveToFileA( LPCSTR lpszFileName )
{
	CxCriticalSection::Owner Lock(*m_pCsLockImage);

	if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
	{
		cvSaveImage( lpszFileName, m_pIPLImage );
		return TRUE;
	}
	return FALSE;
}

BOOL CxImageObject::SaveToFileW( LPCWSTR lpszFileName )
{
	USES_CONVERSION;
	return SaveToFileA( W2A((LPWSTR)lpszFileName) );
}

BOOL CxImageObject::CopyImage( const CxImageObject* pSrcImage )
{
	cvCopy( pSrcImage->GetImage(), m_pIPLImage );
	m_bNotifyChangeImage = TRUE;
	return TRUE;
}

BOOL CxImageObject::Clone( const CxImageObject* pSrcImage )
{
	if ( !pSrcImage ) return FALSE;
	if ( !Create( pSrcImage->GetWidth(), pSrcImage->GetHeight(), pSrcImage->GetBpp(), pSrcImage->GetImage()->origin ) )
		return FALSE;
	m_bNotifyChangeImage = TRUE;
	return CopyImage( pSrcImage );
}

BOOL CxImageObject::CreateFromBuffer( LPVOID lpImgBuf, int nWidth, int nHeight, int nBpp )
{
	CxCriticalSection::Owner Lock(*m_pCsLockImage);

	if ( m_bDelete || !m_pIPLImage || GetBpp() != nBpp || m_pIPLImage->width != nWidth || m_pIPLImage->height != nHeight )
    {
        if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
            Destroy();
    
		m_bDelete = FALSE;

		m_pIPLImage = new IplImage;
		ZeroMemory( m_pIPLImage, sizeof(IplImage) );
		m_pIPLImage->nSize = sizeof(IplImage);
		m_pIPLImage->nChannels = nBpp>>3;
		m_pIPLImage->depth = 8;
		m_pIPLImage->colorModel[0] = 'G'; m_pIPLImage->colorModel[1] = 'R';
		m_pIPLImage->colorModel[2] = 'A'; m_pIPLImage->colorModel[3] = 'Y';
		m_pIPLImage->channelSeq[0] = 'G'; m_pIPLImage->channelSeq[1] = 'R';
		m_pIPLImage->channelSeq[2] = 'A'; m_pIPLImage->channelSeq[3] = 'Y';
		m_pIPLImage->align = 4;
		m_pIPLImage->width = nWidth;
		m_pIPLImage->height = nHeight;
		m_pIPLImage->widthStep = GetWidthBytes(nWidth, nBpp);
 		m_pIPLImage->imageSize = m_pIPLImage->widthStep * nHeight;
    }

	m_pIPLImage->imageData = m_pIPLImage->imageDataOrigin = (char*)lpImgBuf;

	m_bNotifyChangeImage = TRUE;

	return TRUE;
}

BOOL CxImageObject::Create( int nWidth, int nHeight, int nBpp, int nOrigin/*=0*/ )
{
    const unsigned max_img_size = 10000;

    if ( (nBpp != 8 && nBpp != 24 && nBpp != 32) || 
        (nOrigin != IPL_ORIGIN_TL && nOrigin != IPL_ORIGIN_BL) )
    {
        XASSERT( FALSE ); // most probably, it is a programming error
        return FALSE;
    }
    
    if ( !m_bDelete || !m_pIPLImage || GetBpp() != nBpp || m_pIPLImage->width != nWidth || m_pIPLImage->height != nHeight )
    {	
		CxCriticalSection::Owner Lock(*m_pCsLockImage);

        if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
            Destroy();
    
        /* prepare IPL header */
        m_pIPLImage = cvCreateImage( cvSize( nWidth, nHeight ), IPL_DEPTH_8U, nBpp>>3 );
    }

    if ( m_pIPLImage )
        m_pIPLImage->origin = nOrigin == 0 ? IPL_ORIGIN_TL : IPL_ORIGIN_BL;
	
	m_bDelete = TRUE;
	m_bNotifyChangeImage = TRUE;

    return m_pIPLImage ? TRUE : FALSE;
}

void CxImageObject::Destroy()
{
	m_bNotifyChangeImage = TRUE;

	if ( !m_bDelete && m_pIPLImage )
	{
		delete m_pIPLImage;
		m_pIPLImage = NULL;
		return;
	}
	if ( m_pIPLImage != NULL )
	{
		cvReleaseImage( &m_pIPLImage );
	}
}

BYTE CxImageObject::GetPixelLevel( int x, int y ) const
{
	if ( !m_pIPLImage || x >= m_pIPLImage->width || y >= m_pIPLImage->height || x<0 || y<0 ) return 0;
	if ( GetBpp() != 8 ) return 0;

	XASSERT( (y * m_pIPLImage->widthStep + x) < m_pIPLImage->imageSize );
	return m_pIPLImage->imageData[ y * m_pIPLImage->widthStep + x ];
}

COLORREF CxImageObject::GetPixelColor( int x, int y ) const
{
	if ( !m_pIPLImage || x >= m_pIPLImage->width || y >= m_pIPLImage->height || x<0 || y<0 ) return RGB(0,0,0);

	XASSERT( (y * m_pIPLImage->widthStep + x) < m_pIPLImage->imageSize );

	BYTE cR, cG, cB;
	switch ( GetBpp() )
	{
	case 8:
		BYTE cGray;
		cGray = m_pIPLImage->imageData[ y * m_pIPLImage->widthStep + x ];
		return RGB(cGray, cGray, cGray);
	case 16:	// 5 6 5 (r, g, b)
		WORD wValue;
		wValue = m_pIPLImage->imageData[ y*m_pIPLImage->widthStep + x*2+0 ] << 8 | m_pIPLImage->imageData[ y*m_pIPLImage->widthStep + x*2+1 ];
		cB = (BYTE)( (wValue & 0x1F) );
		cG = (BYTE)( (wValue >> 5) & 0x3F );
		cR = (BYTE)( (wValue) >> 11 );

		return RGB(cR, cG, cB);
	case 24:
	case 32:
		cB = m_pIPLImage->imageData[ y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+0 ];
		cG = m_pIPLImage->imageData[ y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+1 ];
		cR = m_pIPLImage->imageData[ y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+2 ];

		return RGB(cR, cG, cB);
	}

	return RGB(0,0,0);
}

FnOnImageProgress CxImageObject::SetOnImageProgress( FnOnImageProgress _fnOnImageProgress )
{ 
	FnOnImageProgress OldProgressFn = m_fnOnImageProgress;
	m_fnOnImageProgress = _fnOnImageProgress; 
	return OldProgressFn;
}