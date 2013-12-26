// xArrowDrawer.cpp: implementation of the CxArrowDrawer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XGraphic/xArrowDrawer.h>
#include <XUtil/String/xString.h>

#include <math.h>
#include <atlconv.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxArrowDrawer::CxArrowDrawer()
{
	m_InnerBrush = NULL;
	m_InnerPen = NULL;
	m_OuterPen = NULL;
}

CxArrowDrawer::~CxArrowDrawer()
{
	Destroy();
}

BOOL CxArrowDrawer::Create( int nPenSize, COLORREF dwInnerPen, COLORREF dwOuterPen, COLORREF dwInnerBrush )
{
	Destroy();

	m_InnerBrush = ::CreateSolidBrush( dwInnerBrush );
	m_InnerPen = ::CreatePen( PS_SOLID, nPenSize, dwInnerPen );
	m_OuterPen = ::CreatePen( PS_SOLID, nPenSize+2, dwOuterPen );

	return TRUE;
}

BOOL CxArrowDrawer::Destroy()
{
	if ( m_InnerBrush )
		::DeleteObject(m_InnerBrush);
	if ( m_InnerPen )
		::DeleteObject(m_InnerPen);
	if ( m_OuterPen )
		::DeleteObject(m_OuterPen);
	return TRUE;
}

BOOL CxArrowDrawer::Draw( HDC hDC, POINT pt1, POINT pt2, ArrowType eArrowType )
{
	HPEN	hOldPen;
	HBRUSH	hOldBrush;

	hOldBrush = (HBRUSH)::SelectObject(hDC, m_InnerBrush);

	double dSlopeY1 , dCosY1 , dSinY1;
	double dSlopeY2 , dCosY2 , dSinY2;
	double dPar = 10.0;		// arrow head size

	dSlopeY1 = atan2( double(pt1.y - pt2.y), double(pt1.x - pt2.x) );
	dCosY1 = cos( dSlopeY1 );
	dSinY1 = sin( dSlopeY1 );

	dSlopeY2 = atan2( double(pt2.y - pt1.y), double(pt2.x - pt1.x) );
	dCosY2 = cos( dSlopeY2 );
	dSinY2 = sin( dSlopeY2 );

	//draw a line between the 2 endpoint
	hOldPen = (HPEN)::SelectObject(hDC, m_OuterPen);
	::MoveToEx( hDC, pt1.x, pt1.y, NULL );
	::LineTo( hDC, pt2.x, pt2.y );
	
	if ( eArrowType & AT_RIGHT )
	{
		::MoveToEx(hDC, pt2.x, pt2.y, NULL);

		::LineTo( hDC, pt2.x + int(dPar * dCosY1 - ( dPar / 2.0 * dSinY1 ) + .5f),
			pt2.y + int(dPar * dSinY1 + ( dPar / 2.0 * dCosY1 ) + .5f) );
		::LineTo( hDC, pt2.x + int(dPar * dCosY1 + dPar / 2.0 * dSinY1 + .5f),
			pt2.y - int(dPar / 2.0 * dCosY1 - dPar * dSinY1 + .5f) );
		::LineTo( hDC, pt2.x, pt2.y );
	}
	
	if ( eArrowType & AT_LEFT )
	{
		::MoveToEx(hDC, pt1.x, pt1.y, NULL);

		::LineTo( hDC, pt1.x + int(dPar * dCosY2 - ( dPar / 2.0 * dSinY2 ) + .5f),
			pt1.y + int(dPar * dSinY2 + ( dPar / 2.0 * dCosY2 ) +.5f) );
		::LineTo( hDC, pt1.x + int(dPar * dCosY2 + dPar / 2.0 * dSinY2 + .5f),
			pt1.y - int(dPar / 2.0 * dCosY2 - dPar * dSinY2 + .5f) );
		::LineTo( hDC, pt1.x, pt1.y );	
	}	
	
	::SelectObject( hDC, m_InnerPen );
	
	::MoveToEx( hDC, pt1.x, pt1.y, NULL );
	::LineTo( hDC, pt2.x, pt2.y );
	
	if ( eArrowType & AT_RIGHT )
	{
		// Draw the arrow head
		::BeginPath(hDC);

		::MoveToEx( hDC, pt2.x, pt2.y, NULL );
		
		::LineTo( hDC, pt2.x + int(dPar * dCosY1 - ( dPar / 2.0 * dSinY1 ) + .5f),
			pt2.y + int(dPar * dSinY1 + ( dPar / 2.0 * dCosY1 ) + .5f) );
		::LineTo( hDC, pt2.x + int(dPar * dCosY1 + dPar / 2.0 * dSinY1 + .5f),
			pt2.y - int(dPar / 2.0 * dCosY1 - dPar * dSinY1 + .5f) );
		::LineTo( hDC, pt2.x, pt2.y );
		
		::EndPath(hDC);

		::StrokeAndFillPath(hDC);
	}
	
	if ( eArrowType & AT_LEFT )
	{
		::BeginPath(hDC);

		::MoveToEx( hDC, pt1.x, pt1.y, NULL );

		::LineTo( hDC, pt1.x + int(dPar * dCosY2 - ( dPar / 2.0 * dSinY2 ) + .5f),
			pt1.y + int(dPar * dSinY2 + ( dPar / 2.0 * dCosY2 ) +.5f) );
		::LineTo( hDC, pt1.x + int(dPar * dCosY2 + dPar / 2.0 * dSinY2 + .5f),
			pt1.y - int(dPar / 2.0 * dCosY2 - dPar * dSinY2 + .5f) );
		::LineTo( hDC, pt1.x, pt1.y );	
		
		::EndPath(hDC);

		::StrokeAndFillPath(hDC);
	}

	::SelectObject( hDC, hOldPen );
	::SelectObject( hDC, hOldBrush );
	
	return TRUE;
}

BOOL CxArrowDrawer::DrawArrowText( HDC hDC, RECT rcText, LPCTSTR lpszText)
{
	int nOldBkMode = ::SetBkMode( hDC, TRANSPARENT );
	DWORD dwOldTextColor = ::SetTextColor( hDC, RGB(0xff, 0xff, 0xff) );

	int nLength = lstrlen(lpszText);
	::OffsetRect(&rcText, 1, 0);
	::DrawText( hDC, lpszText, nLength, &rcText, DT_LEFT|DT_NOCLIP );
	::OffsetRect(&rcText, -2, 0);
	::DrawText( hDC, lpszText, nLength, &rcText, DT_LEFT|DT_NOCLIP );
	::OffsetRect(&rcText, 1, 1);
	::DrawText( hDC, lpszText, nLength, &rcText, DT_LEFT|DT_NOCLIP );
	::OffsetRect(&rcText, 0, -2);
	::DrawText( hDC, lpszText, nLength, &rcText, DT_LEFT|DT_NOCLIP );
	::OffsetRect(&rcText, 0, 1);
	::SetTextColor( hDC, RGB(0,128,128) );
	::DrawText( hDC, lpszText, nLength, &rcText, DT_LEFT|DT_NOCLIP );

	::SetTextColor(hDC, dwOldTextColor);
	::SetBkMode(hDC, nOldBkMode);
	
	return TRUE;
}
