// vDrawDIB.h: interface for the CxDrawDIB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VDRAWDIB_H__55DA5C39_434F_4B38_9332_A04DC7406F53__INCLUDED_)
#define AFX_VDRAWDIB_H__55DA5C39_434F_4B38_9332_A04DC7406F53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxImageObject;
class CxRender2D;
class CxDrawDIB  
{
protected:
	CxRender2D* m_pRenderer;		// Renderer

	BYTE AlphaBlend( BYTE x, BYTE y, BYTE alpha );
	void ScreenScaledBlit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							float fRatioX, float fRatioY,
							int nDstOffsetY=0,
							int nDstOffsetH=0 );
	void ScreenBlit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							int nDstOffsetY=0,
							int nDstOffsetH=0 );

	void Screen32ScaledBlit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							float fRatioX, float fRatioY,
							int nDstOffsetY=0,
							int nDstOffsetH=0 );
	void Screen32Blit( BYTE* pScreenBuffer, const int nScrnWidthBytes, CxImageObject* pImgObj, 
							int nDstX, int nDstY, int nDstOffW, int nDstOffH, 
							int nSrcX, int nSrcY, int nSrcW, int nSrcH,
							int nDstOffsetY=0,
							int nDstOffsetH=0 );

public:
	
	void SetDevice( CxRender2D* pRenderer );
	BOOL Draw( CxImageObject* pImgObj, 
				int nDstX, int nDstY, int nDstW, int nDstH, 
				int nSrcX, int nSrcY, int nSrcW, int nSrcH, 
				float fZoomRatio );

	CxDrawDIB( CxRender2D* pRenderer );
	CxDrawDIB();
	virtual ~CxDrawDIB();

};

inline BYTE CxDrawDIB::AlphaBlend( BYTE x, BYTE y, BYTE alpha )
{
	return (BYTE) ( (( (x) - (y) ) * (alpha) + ((y) << 8)) >> 8 );
}

#endif // !defined(AFX_VDRAWDIB_H__55DA5C39_434F_4B38_9332_A04DC7406F53__INCLUDED_)
