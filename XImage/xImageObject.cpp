/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

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
	m_bNotifyChangeImage(FALSE),
	m_cbOnImageDestroy(NULL),
	m_pOnImageDestroyContext(NULL)
{
	m_pCsLockImage = new CxCriticalSection();
	m_nPixelMaximum = 255;
	m_ChannelSeq = ChannelSeqUnknown;

	m_bUseCustomizedMemory = FALSE;
	m_pCustomizedMemory = NULL;

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

void CxImageObject::SetPixelMaximum( int nValue )
{
	if (!m_pIPLImage)
		return;
	if (m_pIPLImage->depth == 8)
	{
		m_nPixelMaximum = nValue & 0xff;
	}
	else
	{
		m_nPixelMaximum = nValue;
	}
}

int CxImageObject::GetPixelMaximum() const
{
	return m_nPixelMaximum;
}

BOOL CxImageObject::IsValid() const
{
	return m_pIPLImage!=NULL ? TRUE : FALSE;
}

int CxImageObject::GetWidthBytes() const
{
	return !m_pIPLImage ? 0 : m_pIPLImage->widthStep;
}

int CxImageObject::GetWidthBytes( int nCx, int nBitCount, int nAlignBytes/*=4*/ )
{
	return ( ( (nCx * nBitCount + 7) / 8) + nAlignBytes - 1) & (~(nAlignBytes - 1));
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

int CxImageObject::GetChannel() const
{
	return m_pIPLImage ? m_pIPLImage->nChannels : 0;
}

int CxImageObject::GetDepth() const
{
	return m_pIPLImage ? (m_pIPLImage->depth & 255) : 0;
}

int CxImageObject::GetAlignBytes() const
{
	return m_pIPLImage ? (m_pIPLImage->align) : 0;
}

size_t CxImageObject::GetBufferSize() const
{
	return m_pIPLImage ? ((size_t)m_pIPLImage->widthStep * m_pIPLImage->height) : 0;
}

CxImageObject::ChannelSeqModel CxImageObject::GetChannelSeq() const
{
	return m_ChannelSeq;
}

CxCriticalSection*	CxImageObject::GetImageLockObject()
{
	return m_pCsLockImage;
}

struct _IplImage* CxImageObject::GetImage() const
{
	return m_pIPLImage;
}

void CxImageObject::ClearNotifyFlag()
{
	m_bNotifyChangeImage = FALSE;
}

BOOL CxImageObject::IsNotifyFlag()
{
	return m_bNotifyChangeImage;
}

void CxImageObject::SetNotifyFlag()
{
	m_bNotifyChangeImage = TRUE;
}

BOOL CxImageObject::LoadFromFile( LPCTSTR lpszFileName, BOOL bForceGray8/*=FALSE*/ )
{
	USES_CONVERSION;
	CxCriticalSection::Owner Lock(*m_pCsLockImage);
	//if ( !m_bDelete )
	Destroy();

	try
	{
		m_pIPLImage = cvLoadImage( T2A((LPTSTR)lpszFileName), bForceGray8 ? CV_LOAD_IMAGE_GRAYSCALE : CV_LOAD_IMAGE_ANYDEPTH|CV_LOAD_IMAGE_ANYCOLOR );	// 2nd parameter: 0-gray, 1-color
		if (!m_pIPLImage)
			return FALSE;

		if ((m_pIPLImage->channelSeq[0] == 'G') && (m_pIPLImage->channelSeq[1] == 'R') && 
			(m_pIPLImage->channelSeq[2] == 'A') && (m_pIPLImage->channelSeq[3] == 'Y'))
		{
			m_ChannelSeq = ChannelSeqGray;
		}
		else if ((m_pIPLImage->channelSeq[0] == 'B') && (m_pIPLImage->channelSeq[1] == 'G') && 
			(m_pIPLImage->channelSeq[2] == 'R'))
		{
			m_ChannelSeq = ChannelSeqBGR;
		}
		else if ((m_pIPLImage->channelSeq[0] == 'R') && (m_pIPLImage->channelSeq[1] == 'G') && 
			(m_pIPLImage->channelSeq[2] == 'B'))
		{
			m_ChannelSeq = ChannelSeqRGB;
		}
		else
		{
			m_ChannelSeq = ChannelSeqUnknown;
		}
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

BOOL CxImageObject::SaveToFile( LPCTSTR lpszFileName )
{
	USES_CONVERSION;
	CxCriticalSection::Owner Lock(*m_pCsLockImage);

	if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
	{
		if (m_pIPLImage->nChannels == 3)
		{
			if (m_pIPLImage->depth == 8)
			{
				if (m_ChannelSeq == ChannelSeqRGB)
				{
					BYTE cTemp;
					for (int j=0 ; j<m_pIPLImage->height ; j++)
					{
						for (int i=0 ; i<m_pIPLImage->width*3 ; i+=3)
						{
							cTemp = m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 0];
							m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 0] = m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 2];
							m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 2] = cTemp;
						}
					}
				}
			}
			else if (m_pIPLImage->depth == 16)
			{
				// TODO: 16bit color image
			}
		}
		int ret = cvSaveImage( T2A((LPTSTR)lpszFileName), m_pIPLImage );
		if (m_pIPLImage->nChannels == 3)
		{
			if (m_pIPLImage->depth == 8)
			{
				if (m_ChannelSeq == ChannelSeqRGB)
				{
					BYTE cTemp;
					for (int j=0 ; j<m_pIPLImage->height ; j++)
					{
						for (int i=0 ; i<m_pIPLImage->width*3 ; i+=3)
						{
							cTemp = m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 0];
							m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 0] = m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 2];
							m_pIPLImage->imageData[j*m_pIPLImage->widthStep + i + 2] = cTemp;
						}
					}
				}
			}
			else if (m_pIPLImage->depth == 16)
			{
				// TODO: 16bit color image
			}
		}
		return ret != 0 ? TRUE : FALSE;
	}
	return FALSE;
}

BOOL CxImageObject::CopyImage( const CxImageObject* pSrcImage )
{
	if (!pSrcImage || !pSrcImage->IsValid())
		return FALSE;
	if (!IsValid())
		return FALSE;
	if ((GetWidth() != pSrcImage->GetWidth()) ||
		(GetHeight() != pSrcImage->GetHeight()) ||
		(GetDepth() != pSrcImage->GetDepth()) ||
		(GetChannel() != pSrcImage->GetChannel()) ||
		(GetChannelSeq() != pSrcImage->GetChannelSeq()) ||
		(GetAlignBytes() != pSrcImage->GetAlignBytes()))
		return FALSE;

	BYTE* pSrcBuf = (BYTE*)pSrcImage->GetImageBuffer();
	BYTE* pDstBuf = (BYTE*)GetImageBuffer();
	int nSrcWBytes = pSrcImage->GetWidthBytes();
	int nDstWBytes = GetWidthBytes();
	int nWBytes = nSrcWBytes > nDstWBytes ? nDstWBytes : nSrcWBytes;

	for (unsigned int y=0 ; y<(unsigned int)GetHeight() ; y++)
	{
		memcpy( pDstBuf+y*nDstWBytes, pSrcBuf+y*nSrcWBytes, nWBytes );
	}

	m_ChannelSeq = pSrcImage->GetChannelSeq();
	m_bNotifyChangeImage = TRUE;
	return TRUE;
}

BOOL CxImageObject::ChangeBufferAlignment( int nAlignBytes )
{
	if (!m_bDelete)
	{
		XTRACE( _T("CxImageObject::ChangeBufferAlignment - DO NOT USE EXTERNAL MEMORY!\n") );
		return FALSE;
	}

	if (GetAlignBytes() == nAlignBytes)
		return TRUE;
	if (GetWidthBytes() == CxImageObject::GetWidthBytes(GetWidth(), GetDepth()*GetChannel(), nAlignBytes))
	{
		m_pIPLImage->align = nAlignBytes;
		return TRUE;
	}

	CxImageObject ImageTemp;
	ImageTemp.Create(GetWidth(), GetHeight(), GetDepth(), GetChannel(), 0, GetChannelSeq(), nAlignBytes);

	BYTE* pSrcBuf = (BYTE*)GetImageBuffer();
	BYTE* pDstBuf = (BYTE*)ImageTemp.GetImageBuffer();
	int nSrcWBytes = GetWidthBytes();
	int nDstWBytes = ImageTemp.GetWidthBytes();
	int nWBytes = nSrcWBytes > nDstWBytes ? nDstWBytes : nSrcWBytes;

	for (unsigned int y=0 ; y<(unsigned int)GetHeight() ; y++)
	{
		memcpy( pDstBuf+y*nDstWBytes, pSrcBuf+y*nSrcWBytes, nWBytes );
	}

	int nWidth = GetWidth();
	int nHeight = GetHeight();
	int nDepth = GetDepth();
	int nChannel = GetChannel();
	ChannelSeqModel seq = GetChannelSeq();
	Destroy();
	Clone(&ImageTemp);

	return TRUE;
}

BOOL CxImageObject::Clone( const CxImageObject* pSrcImage )
{
	if ( !pSrcImage ) return FALSE;
	if ( !Create( pSrcImage->GetWidth(), pSrcImage->GetHeight(), pSrcImage->GetDepth(), pSrcImage->GetChannel(), pSrcImage->GetImage()->origin, pSrcImage->GetChannelSeq(), pSrcImage->GetAlignBytes() ) )
		return FALSE;
	m_bNotifyChangeImage = TRUE;
	return CopyImage( pSrcImage );
}

BOOL CxImageObject::CreateFromBuffer( LPVOID lpImgBuf, int nWidth, int nHeight, int nDepth, int nChannel, 
									 ChannelSeqModel seq/*=ChannelSeqUnknown*/, int nAlignBytes/*=4*/,
									 CbOnImageDestroy cbOnDestroy/*=NULL*/, LPVOID lpContext/*=NULL*/)
{
    if ( (nDepth != 8 && nDepth != 16) || 
		(nChannel != 1 && nChannel != 3) )
    {
        XASSERT( FALSE ); // most probably, it is a programming error
        return FALSE;
    }

	CxCriticalSection::Owner Lock(*m_pCsLockImage);

	if ( m_bDelete || !m_pIPLImage || 
		GetChannel() != nChannel || 
		GetDepth() != nDepth || 
		m_pIPLImage->width != nWidth || m_pIPLImage->height != nHeight ||
		m_pIPLImage->align != nAlignBytes )
    {
        if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
            Destroy();

		m_cbOnImageDestroy = cbOnDestroy;
		m_pOnImageDestroyContext = lpContext;
    
		m_bDelete = FALSE;

		m_pIPLImage = new IplImage;
		ZeroMemory( m_pIPLImage, sizeof(IplImage) );
		m_pIPLImage->nSize = sizeof(IplImage);
		m_pIPLImage->nChannels = nChannel;
		m_pIPLImage->depth = nDepth;
		m_pIPLImage->colorModel[0] = 'G'; m_pIPLImage->colorModel[1] = 'R';
		m_pIPLImage->colorModel[2] = 'A'; m_pIPLImage->colorModel[3] = 'Y';
		if (nChannel == 1)
		{
			m_ChannelSeq = ChannelSeqGray;
			m_pIPLImage->channelSeq[0] = 'G'; m_pIPLImage->channelSeq[1] = 'R';
			m_pIPLImage->channelSeq[2] = 'A'; m_pIPLImage->channelSeq[3] = 'Y';
		}
		else
		{
			switch (seq)
			{
			case ChannelSeqGray:
			case ChannelSeqBGR:
			case ChannelSeqUnknown:
				m_ChannelSeq = ChannelSeqBGR;
				m_pIPLImage->channelSeq[0] = 'B'; m_pIPLImage->channelSeq[1] = 'G';
				m_pIPLImage->channelSeq[2] = 'R'; m_pIPLImage->channelSeq[3] = '\0';
				break;
			case ChannelSeqRGB:
				m_ChannelSeq = ChannelSeqRGB;
				m_pIPLImage->channelSeq[0] = 'R'; m_pIPLImage->channelSeq[1] = 'G';
				m_pIPLImage->channelSeq[2] = 'B'; m_pIPLImage->channelSeq[3] = '\0';
				break;
			}
		}
		m_pIPLImage->align = nAlignBytes;
		m_pIPLImage->width = nWidth;
		m_pIPLImage->height = nHeight;
		m_pIPLImage->widthStep = GetWidthBytes(nWidth, nDepth*nChannel, nAlignBytes);
 		m_pIPLImage->imageSize = m_pIPLImage->widthStep * nHeight;

	    m_bNotifyChangeImage = TRUE;
    }

	m_pIPLImage->imageData = m_pIPLImage->imageDataOrigin = (char*)lpImgBuf;

	return TRUE;
}

void CxImageObject::CreateFromHBitmap( HBITMAP hBitmap )
{
	CxCriticalSection::Owner Lock(*m_pCsLockImage);

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

	int nDestW = bm.bmWidth;
	int nDestH = bm.bmHeight;

	if ( cClrBits == 8 )
		Create( nDestW, nDestH, 8, 1, 0, ChannelSeqGray );
	else
		Create( nDestW, nDestH, 8, 3, 0, ChannelSeqBGR );

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
}


BOOL CxImageObject::Create( int nWidth, int nHeight, int nDepth, int nChannel, int nOrigin/*=0*/, ChannelSeqModel seq/*=ChannelSeqUnknown*/, int nAlignBytes/*=4*/ )
{
    if ( (nDepth != 8 && nDepth != 16) || 
		(nChannel != 1 && nChannel != 3) ||
        (nOrigin != IPL_ORIGIN_TL && nOrigin != IPL_ORIGIN_BL) )
    {
        XASSERT( FALSE ); // most probably, it is a programming error
        return FALSE;
    }

	if (nWidth == 0 || nHeight == 0)
		return FALSE;

	if (seq == ChannelSeqUnknown)
		seq = m_ChannelSeq;
    
    if ( !m_bDelete || !m_pIPLImage || 
		GetChannel() != nChannel || 
		GetDepth() != nDepth || 
		m_pIPLImage->width != nWidth || m_pIPLImage->height != nHeight ||
		(m_ChannelSeq != seq) )
    {
		CxCriticalSection::Owner Lock(*m_pCsLockImage);

        if ( m_pIPLImage && m_pIPLImage->nSize == sizeof(IplImage) )
            Destroy();

		m_cbOnImageDestroy = NULL;
		m_pOnImageDestroyContext = NULL;

		m_bUseCustomizedMemory = FALSE;
		m_pCustomizedMemory = NULL;

		//if (((DWORD)GetWidthBytes(nWidth, nDepth*nChannel, nAlignBytes)*nHeight > 0x7FFFFFFF) || (nAlignBytes != 4))
		{
			m_pIPLImage = new IplImage;
			ZeroMemory( m_pIPLImage, sizeof(IplImage) );
			m_pIPLImage->nSize = sizeof(IplImage);
			m_pIPLImage->nChannels = nChannel;
			m_pIPLImage->depth = nDepth;
			m_pIPLImage->colorModel[0] = 'G'; m_pIPLImage->colorModel[1] = 'R';
			m_pIPLImage->colorModel[2] = 'A'; m_pIPLImage->colorModel[3] = 'Y';
			if (nChannel == 1)
			{
				m_ChannelSeq = ChannelSeqGray;
				m_pIPLImage->channelSeq[0] = 'G'; m_pIPLImage->channelSeq[1] = 'R';
				m_pIPLImage->channelSeq[2] = 'A'; m_pIPLImage->channelSeq[3] = 'Y';
			}
			else
			{
				switch (seq)
				{
				case ChannelSeqGray:
				case ChannelSeqBGR:
				case ChannelSeqUnknown:
					m_ChannelSeq = ChannelSeqBGR;
					m_pIPLImage->channelSeq[0] = 'B'; m_pIPLImage->channelSeq[1] = 'G';
					m_pIPLImage->channelSeq[2] = 'R'; m_pIPLImage->channelSeq[3] = '\0';
					break;
				case ChannelSeqRGB:
					m_ChannelSeq = ChannelSeqRGB;
					m_pIPLImage->channelSeq[0] = 'R'; m_pIPLImage->channelSeq[1] = 'G';
					m_pIPLImage->channelSeq[2] = 'B'; m_pIPLImage->channelSeq[3] = '\0';
					break;
				}
			}
			m_pIPLImage->align = nAlignBytes;
			m_pIPLImage->width = nWidth;
			m_pIPLImage->height = nHeight;
			m_pIPLImage->widthStep = GetWidthBytes(nWidth, nDepth*nChannel, nAlignBytes);
			m_pIPLImage->imageSize = m_pIPLImage->widthStep * nHeight;

			m_bUseCustomizedMemory = TRUE;
			m_pCustomizedMemory = (BYTE*) malloc( (size_t)m_pIPLImage->widthStep * m_pIPLImage->height );
			XTRACE( _T("CxImageObject::Create CustomizedMemory! %p - %d bytes\n"), m_pCustomizedMemory, (size_t)m_pIPLImage->widthStep * m_pIPLImage->height );

			if (!m_pCustomizedMemory)
			{
				m_bUseCustomizedMemory = FALSE;
				delete m_pIPLImage;
				m_pIPLImage = NULL;
				return FALSE;
			}

			m_pIPLImage->imageData = m_pIPLImage->imageDataOrigin = (char*)m_pCustomizedMemory;
		}
		/*else
		{
			try
			{
				m_pIPLImage = cvCreateImage( cvSize( nWidth, nHeight ), nDepth, nChannel );

				if (nChannel == 1)
				{
					m_ChannelSeq = ChannelSeqGray;
					m_pIPLImage->channelSeq[0] = 'G'; m_pIPLImage->channelSeq[1] = 'R';
					m_pIPLImage->channelSeq[2] = 'A'; m_pIPLImage->channelSeq[3] = 'Y';
				}
				else
				{
					switch (seq)
					{
					case ChannelSeqGray:
					case ChannelSeqBGR:
					case ChannelSeqUnknown:
						m_ChannelSeq = ChannelSeqBGR;
						m_pIPLImage->channelSeq[0] = 'B'; m_pIPLImage->channelSeq[1] = 'G';
						m_pIPLImage->channelSeq[2] = 'R'; m_pIPLImage->channelSeq[3] = '\0';
						break;
					case ChannelSeqRGB:
						m_ChannelSeq = ChannelSeqRGB;
						m_pIPLImage->channelSeq[0] = 'R'; m_pIPLImage->channelSeq[1] = 'G';
						m_pIPLImage->channelSeq[2] = 'B'; m_pIPLImage->channelSeq[3] = '\0';
						break;
					}
				}
			}
			catch( cv::Exception& e )
			{
				const char* err_msg = e.what();
				CxString strX;
				strX = _T("CxImageObject::Create - exception caught: ");
				strX += err_msg;
				strX += _T("\n");
				XTRACE( strX );
			}
		}*/
	    m_bNotifyChangeImage = TRUE;
    }

    if ( m_pIPLImage )
        m_pIPLImage->origin = nOrigin == 0 ? IPL_ORIGIN_TL : IPL_ORIGIN_BL;
	
	m_bDelete = TRUE;

    return m_pIPLImage ? TRUE : FALSE;
}

void CxImageObject::Destroy()
{
	m_bNotifyChangeImage = TRUE;

	if ( m_pCustomizedMemory )
	{
		free (m_pCustomizedMemory);
		m_pCustomizedMemory = NULL;

		if (m_pIPLImage)
		{
			delete m_pIPLImage;
			m_pIPLImage = NULL;
		}

		m_bUseCustomizedMemory = FALSE;
		return;
	}

	if ( !m_bDelete && m_pIPLImage )
	{
		if (m_cbOnImageDestroy)
		{
			m_cbOnImageDestroy(m_pIPLImage->imageData, m_pOnImageDestroyContext);
		}
		delete m_pIPLImage;
		m_pIPLImage = NULL;
		m_pCustomizedMemory = NULL;
		m_pOnImageDestroyContext = NULL;
		m_cbOnImageDestroy = NULL;
		return;
	}
	if ( m_pIPLImage != NULL )
	{
		cvReleaseImage( &m_pIPLImage );
	}
}

unsigned int CxImageObject::GetPixelLevel( int x, int y ) const
{
	if ( !m_pIPLImage || x >= m_pIPLImage->width || y >= m_pIPLImage->height || x<0 || y<0 ) return 0;
	if ( m_pIPLImage->nChannels != 1 ) return 0;

	XASSERT( (unsigned int)(y * m_pIPLImage->widthStep + x*(GetBpp()>>3)) < (unsigned int)m_pIPLImage->imageSize );

	if (m_pIPLImage->depth == 8)
	{
		return (unsigned char)m_pIPLImage->imageData[ (unsigned int)y * m_pIPLImage->widthStep + x ];
	}
	else if (m_pIPLImage->depth == 16)
	{
		return *(unsigned short*)(m_pIPLImage->imageData + (unsigned int)y * m_pIPLImage->widthStep + x*2);
	}
	return 0;
}

COLORREF CxImageObject::GetPixelColor( int x, int y ) const
{
	if ( !m_pIPLImage || x >= m_pIPLImage->width || y >= m_pIPLImage->height || x<0 || y<0 ) return RGB(0,0,0);

	XASSERT( (unsigned int)(y * m_pIPLImage->widthStep + x*(GetBpp()>>3)) < (unsigned int)m_pIPLImage->imageSize );

	BYTE cR, cG, cB;
	switch ( GetBpp() )
	{
	case 8:
		BYTE cGray;
		cGray = m_pIPLImage->imageData[ (unsigned int)y * m_pIPLImage->widthStep + x ];
		return RGB(cGray, cGray, cGray);
	case 16:	// 5 6 5 (r, g, b)
		if (GetChannel() == 3)
		{
			WORD wValue;
			wValue = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*2+0 ] << 8 | m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*2+1 ];
			if ((m_ChannelSeq == ChannelSeqBGR) || (m_ChannelSeq == ChannelSeqUnknown))
			{
				cB = (BYTE)( (wValue & 0x1F) );
				cG = (BYTE)( (wValue >> 5) & 0x3F );
				cR = (BYTE)( (wValue) >> 11 );
			}
			else
			{
				cR = (BYTE)( (wValue & 0x1F) );
				cG = (BYTE)( (wValue >> 5) & 0x3F );
				cB = (BYTE)( (wValue) >> 11 );
			}

			return RGB(cR, cG, cB);
		}
		else // 16bit 1channel
		{
			unsigned short nGray = *(unsigned short*)(m_pIPLImage->imageData + (unsigned int)y * m_pIPLImage->widthStep + x*2);
			nGray = nGray * 255 / m_nPixelMaximum;
			return RGB(nGray, nGray, nGray);
		}
	case 24:
	case 32:
		if ((m_ChannelSeq == ChannelSeqBGR) || (m_ChannelSeq == ChannelSeqUnknown))
		{
			cB = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+0 ];
			cG = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+1 ];
			cR = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+2 ];
		}
		else
		{
			cR = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+0 ];
			cG = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+1 ];
			cB = m_pIPLImage->imageData[ (unsigned int)y*m_pIPLImage->widthStep + x*m_pIPLImage->nChannels+2 ];
		}

		return RGB(cR, cG, cB);
	}

	return RGB(0,0,0);
}
