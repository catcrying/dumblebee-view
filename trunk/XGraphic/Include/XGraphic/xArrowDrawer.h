// ArrowDrawer.h: interface for the CxArrowDrawer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARROWDRAWER_H__D6A312EC_AF83_4D57_B2A4_3BBF7B000985__INCLUDED_)
#define AFX_ARROWDRAWER_H__D6A312EC_AF83_4D57_B2A4_3BBF7B000985__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <XGraphic/export.h>
#include <wtypes.h>

class XGRAPHIC_API CxArrowDrawer  
{
public:
	enum ArrowType { AT_NONE=0x00, AT_BOTH=0x03, AT_LEFT=0x01, AT_RIGHT=0x02 };
protected:
	HPEN		m_InnerPen;
	HPEN		m_OuterPen;
	HBRUSH		m_InnerBrush;
public:
	CxArrowDrawer();
	virtual ~CxArrowDrawer();

	BOOL Create( int nPenSize, COLORREF dwInnerPen, COLORREF dwOuterPen, COLORREF dwInnerBrush );
	BOOL Destroy();

	BOOL Draw( HDC hDC, POINT pt1, POINT pt2, ArrowType eArrowType );

	BOOL DrawArrowTextA( HDC hDC, RECT rcText, LPCSTR lpszText);
	BOOL DrawArrowTextW( HDC hDC, RECT rcText, LPCWSTR lpszText);

#ifdef _UNICODE
	#define DrawArrowText DrawArrowTextW
#else
	#define DrawArrowText DrawArrowTextA
#endif

};

#endif // !defined(AFX_ARROWDRAWER_H__D6A312EC_AF83_4D57_B2A4_3BBF7B000985__INCLUDED_)
