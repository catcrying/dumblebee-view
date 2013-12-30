#ifndef __X_IMAGE_OBJECT_H__
#define __X_IMAGE_OBJECT_H__

#ifdef XIMAGE_EXPORTS
#define XIMAGE_API	__declspec(dllexport)
#else
#define XIMAGE_API	__declspec(dllimport)
#endif

#include <wtypes.h>
#include <tchar.h>

typedef void (APIENTRY* FnOnImageProgress)( int nProgress );

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxCriticalSection;
struct _IplImage;
class XIMAGE_API CxImageObject  
{
protected:
	struct _IplImage*	m_pIPLImage;
	BOOL			m_bDelete;
	DWORD_PTR		m_dwUsrData1;
	DWORD_PTR		m_dwUsrData2;
	DWORD_PTR		m_dwUsrData3;
	int				m_nPixelMaximum;

	BOOL			m_bNotifyChangeImage;

	FnOnImageProgress	m_fnOnImageProgress;

	CxCriticalSection*	m_pCsLockImage;

	HBITMAP			m_hBitmap;

public:
	CxImageObject();
	virtual ~CxImageObject();

	static void CDECL _OnProgress( int nProgress, LPVOID lpUsrData );

	virtual void OnProgress( int nProgress ) {}
	FnOnImageProgress SetOnImageProgress( FnOnImageProgress _fnOnImageProgress );

	// User-defined data In/Out
	void SetData1( DWORD_PTR dwUsrData1 );
	DWORD_PTR GetData1();
	void SetData2( DWORD_PTR dwUsrData2 );
	DWORD_PTR GetData2();
	void SetData3( DWORD_PTR dwUsrData3 );
	DWORD_PTR GetData3();

	void SetPixelMaximum( int nValue );
	int GetPixelMaximum() const;

	// HBITMAP handling
	void AttachHBitmap( HBITMAP hBitmap );
	void DetachHBitmap();
	BOOL IsHBitmapAttached();

	// Validation
	BOOL IsValid() const;

	// Construction
	BOOL CreateFromBuffer( LPVOID lpImgBuf, int nWidth, int nHeight, int nDepth, int nChannel ); // lpImgBuf must be 4-aligned
	BOOL Create( int nWidth, int nHeight, int nDepth, int nChannel, int nOrigin=0 );
	void Destroy();

	virtual BOOL LoadFromFile( LPCTSTR lpszFileName, BOOL bForceGray8=FALSE );
	virtual BOOL SaveToFile( LPCTSTR lpszFileName );

	BOOL Clone( const CxImageObject* pSrcImage );

	BOOL CopyImage( const CxImageObject* pSrcImage );

	// Retrieve Information
    int GetWidth() const;
    int GetHeight() const;
    int GetBpp() const;
	int GetDepth() const;
	int GetChannel() const;

	static int GetWidthBytes( int nCx, int nBitCount );	// only 4-byte align
	int GetWidthBytes() const;

	// Access Image-Buffer
	LPVOID GetImageBuffer() const;
	unsigned int GetPixelLevel( int x, int y ) const;
	COLORREF GetPixelColor( int x, int y ) const;

	struct _IplImage* GetImage() const;

	CxImageObject(const CxImageObject &rhs);
	CxImageObject &operator=(const CxImageObject &rhs);

	void ClearNotifyFlag();
	BOOL IsNotifyFlag();

	CxCriticalSection*	GetImageLockObject();
};

#endif // __X_IMAGE_OBJECT_H__
