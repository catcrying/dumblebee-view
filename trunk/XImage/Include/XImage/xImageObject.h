/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#ifndef __X_IMAGE_OBJECT_H__
#define __X_IMAGE_OBJECT_H__

#ifdef XIMAGE_EXPORTS
#define XIMAGE_API	__declspec(dllexport)
#else
#define XIMAGE_API	__declspec(dllimport)
#endif

#include <wtypes.h>
#include <tchar.h>

typedef void (APIENTRY* CbOnImageDestroy)(LPVOID lpImgBuf, LPVOID lpContext);

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxCriticalSection;
struct _IplImage;
class XIMAGE_API CxImageObject  
{
public:
	enum ChannelSeqModel { ChannelSeqUnknown, ChannelSeqGray, ChannelSeqBGR, ChannelSeqRGB };
protected:
	struct _IplImage*	m_pIPLImage;
	BOOL			m_bDelete;
	DWORD_PTR		m_dwUsrData1;
	DWORD_PTR		m_dwUsrData2;
	DWORD_PTR		m_dwUsrData3;
	int				m_nPixelMaximum;

	BOOL			m_bNotifyChangeImage;

	CxCriticalSection*	m_pCsLockImage;

	ChannelSeqModel	m_ChannelSeq;

	BOOL			m_bUseCustomizedMemory;
	BYTE*			m_pCustomizedMemory;

	CbOnImageDestroy	m_cbOnImageDestroy;
	LPVOID				m_pOnImageDestroyContext;

public:
	CxImageObject();
	virtual ~CxImageObject();

	// User-defined data In/Out
	void SetData1( DWORD_PTR dwUsrData1 );
	DWORD_PTR GetData1();
	void SetData2( DWORD_PTR dwUsrData2 );
	DWORD_PTR GetData2();
	void SetData3( DWORD_PTR dwUsrData3 );
	DWORD_PTR GetData3();

	void SetPixelMaximum( int nValue );
	int GetPixelMaximum() const;

	// Validation
	BOOL IsValid() const;

	// Construction
	BOOL CreateFromBuffer( LPVOID lpImgBuf, int nWidth, int nHeight, int nDepth, int nChannel, 
		ChannelSeqModel seq=ChannelSeqUnknown, int nAlignBytes=4, 
		CbOnImageDestroy cbOnDestroy=NULL, LPVOID lpContext=NULL );
	void CreateFromHBitmap( HBITMAP hBitmap );
	BOOL Create( int nWidth, int nHeight, int nDepth, int nChannel, int nOrigin=0, ChannelSeqModel seq=ChannelSeqUnknown, int nAlignBytes=4 );

	void Destroy();

	virtual BOOL LoadFromFile( LPCTSTR lpszFileName, BOOL bForceGray8=FALSE );
	virtual BOOL SaveToFile( LPCTSTR lpszFileName );

	BOOL Clone( const CxImageObject* pSrcImage );

	BOOL CopyImage( const CxImageObject* pSrcImage );

	BOOL ChangeBufferAlignment( int nAlignBytes );

	// Retrieve Information
    int GetWidth() const;
    int GetHeight() const;
    int GetBpp() const;
	int GetDepth() const;
	int GetAlignBytes() const;
	int GetChannel() const;
	size_t GetBufferSize() const;

	static int GetWidthBytes( int nCx, int nBitCount, int nAlignBytes=4 );
	int GetWidthBytes() const;
	ChannelSeqModel GetChannelSeq() const;

	// Access Image-Buffer
	LPVOID GetImageBuffer() const;
	unsigned int GetPixelLevel( int x, int y ) const;
	COLORREF GetPixelColor( int x, int y ) const;

	struct _IplImage* GetImage() const;

	CxImageObject(const CxImageObject &rhs);
	CxImageObject &operator=(const CxImageObject &rhs);

	void ClearNotifyFlag();
	BOOL IsNotifyFlag();
	void SetNotifyFlag();

	CxCriticalSection*	GetImageLockObject();
};

#endif // __X_IMAGE_OBJECT_H__
