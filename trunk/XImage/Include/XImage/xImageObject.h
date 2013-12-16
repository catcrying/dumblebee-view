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

	// HBITMAP handling
	void AttachHBitmap( HBITMAP hBitmap );
	void DetachHBitmap();
	BOOL IsHBitmapAttached();

	// Validation
	BOOL IsValid() const;

	// Construction
	BOOL CreateFromBuffer( LPVOID lpImgBuf, int nWidth, int nHeight, int nBpp ); // lpImgBuf must be 4-aligned
	BOOL Create( int nWidth, int nHeight, int nBpp, int nOrigin=0 );
	void Destroy();

	virtual BOOL LoadFromFileW( LPCWSTR lpszFileName, BOOL bForceGray8=FALSE );
	virtual BOOL LoadFromFileA( LPCSTR lpszFileName, BOOL bForceGray8=FALSE );
	virtual BOOL SaveToFileW( LPCWSTR lpszFileName );
	virtual BOOL SaveToFileA( LPCSTR lpszFileName );

#ifdef _UNICODE
	#define LoadFromFile	LoadFromFileW
	#define SaveToFile		SaveToFileW
#else
	#define LoadFromFile	LoadFromFileA
	#define SaveToFile		SaveToFileA
#endif

	BOOL Clone( const CxImageObject* pSrcImage );

	BOOL CopyImage( const CxImageObject* pSrcImage );

	// Retrieve Information
    int GetWidth() const;
    int GetHeight() const;
    int GetBpp() const;

	static int GetWidthBytes( int nCx, int nBitCount );
	int GetWidthBytes() const;

	// Access Image-Buffer
	LPVOID GetImageBuffer() const;
	BYTE GetPixelLevel( int x, int y ) const;
	COLORREF GetPixelColor( int x, int y ) const;

	struct _IplImage* GetImage() const;

	CxImageObject(const CxImageObject &rhs);
	CxImageObject &operator=(const CxImageObject &rhs);

	void ClearNotifyFlag();
	BOOL IsNotifyFlag();

	CxCriticalSection*	GetImageLockObject();
};

#endif // __X_IMAGE_OBJECT_H__
