/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"
#include "ColorRectTracker.h"

AFX_STATIC_DATA HCURSOR _afxCursors[10] = { 0, };
AFX_STATIC_DATA HBRUSH _afxHatchBrush = 0;
AFX_STATIC_DATA HPEN _afxBlackDottedPen = 0;
AFX_STATIC_DATA int _afxHandleSize = 0;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CColorRectTracker::CColorRectTracker()
{
	m_WhiteBoldPen.CreatePen( PS_SOLID, 3, RGB(0xff,0xff,150) );
}

CColorRectTracker::~CColorRectTracker()
{
	m_WhiteBoldPen.DeleteObject();
}

void CColorRectTracker::Draw(CDC* pDC)
{
	// set initial DC state
	VERIFY(pDC->SaveDC() != 0);
	pDC->SetMapMode(MM_TEXT);
	pDC->SetViewportOrg(0, 0);
	pDC->SetWindowOrg(0, 0);

	// get normalized rectangle
	CRect rect = m_rect;
	rect.NormalizeRect();

	CPen* pOldPen = NULL;
	CBrush* pOldBrush = NULL;
	CGdiObject* pTemp;
	int nOldROP;
	
	CPoint ptCenter = rect.CenterPoint();
	int nCrossHairWH = rect.Width() < rect.Height() ? rect.Width()/4 : rect.Height()/4;
	if (nCrossHairWH > 20) nCrossHairWH = 20;
	if (nCrossHairWH < 4) nCrossHairWH = 0;

	// draw lines
	if ((m_nStyle & (dottedLine|solidLine)) != 0)
	{
		rect.InflateRect(+1, +1);   // borders are one pixel outside
		pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
		nOldROP = pDC->SetROP2(R2_COPYPEN);

		pOldPen = pDC->SelectObject( &m_WhiteBoldPen );
		pDC->Rectangle(rect.left, rect.top, rect.right, rect.bottom);
		
		if (nCrossHairWH > 0)
		{
			pDC->MoveTo(ptCenter.x-nCrossHairWH/2, ptCenter.y);
			pDC->LineTo(ptCenter.x+nCrossHairWH/2, ptCenter.y);
			pDC->MoveTo(ptCenter.x, ptCenter.y-nCrossHairWH/2);
			pDC->LineTo(ptCenter.x, ptCenter.y+nCrossHairWH/2);
		}
		
		if (m_nStyle & dottedLine)
			pDC->SelectObject(CPen::FromHandle(_afxBlackDottedPen));
		else
			pDC->SelectStockObject(BLACK_PEN);
		pDC->Rectangle(rect.left, rect.top, rect.right, rect.bottom);

		if (nCrossHairWH > 0)
		{
			pDC->MoveTo(ptCenter.x-nCrossHairWH/2, ptCenter.y);
			pDC->LineTo(ptCenter.x+nCrossHairWH/2, ptCenter.y);
			pDC->MoveTo(ptCenter.x, ptCenter.y-nCrossHairWH/2);
			pDC->LineTo(ptCenter.x, ptCenter.y+nCrossHairWH/2);
		}

		pDC->SetROP2(nOldROP);
	}

	// if hatchBrush is going to be used, need to unrealize it
	if ((m_nStyle & (hatchInside|hatchedBorder)) != 0)
		UnrealizeObject(_afxHatchBrush);

	// hatch inside
	if ((m_nStyle & hatchInside) != 0)
	{
		pTemp = pDC->SelectStockObject(NULL_PEN);
		if (pOldPen == NULL)
			pOldPen = (CPen*)pTemp;
		pTemp = pDC->SelectObject(CBrush::FromHandle(_afxHatchBrush));
		if (pOldBrush == NULL)
			pOldBrush = (CBrush*)pTemp;
		pDC->SetBkMode(TRANSPARENT);
		nOldROP = pDC->SetROP2(R2_MASKNOTPEN);
		pDC->Rectangle(rect.left+1, rect.top+1, rect.right, rect.bottom);
		pDC->SetROP2(nOldROP);
	}

	// draw hatched border
	if ((m_nStyle & hatchedBorder) != 0)
	{
		pTemp = pDC->SelectObject(CBrush::FromHandle(_afxHatchBrush));
		if (pOldBrush == NULL)
			pOldBrush = (CBrush*)pTemp;
		pDC->SetBkMode(OPAQUE);
		CRect rectTrue;
		GetTrueRect(&rectTrue);
		pDC->PatBlt(rectTrue.left, rectTrue.top, rectTrue.Width(),
			rect.top-rectTrue.top, 0x000F0001 /* Pn */);
		pDC->PatBlt(rectTrue.left, rect.bottom,
			rectTrue.Width(), rectTrue.bottom-rect.bottom, 0x000F0001 /* Pn */);
		pDC->PatBlt(rectTrue.left, rect.top, rect.left-rectTrue.left,
			rect.Height(), 0x000F0001 /* Pn */);
		pDC->PatBlt(rect.right, rect.top, rectTrue.right-rect.right,
			rect.Height(), 0x000F0001 /* Pn */);
	}

	// draw resize handles
	if ((m_nStyle & (resizeInside|resizeOutside)) != 0)
	{
		UINT mask = GetHandleMask();
		for (int i = 0; i < 8; ++i)
		{
			if (mask & (1<<i))
			{
				GetHandleRect((TrackerHit)i, &rect);
				rect.InflateRect( +1, +1, +1, +1 );
				pDC->FillSolidRect(rect, RGB(0xff, 0xff, 0xff));
				rect.DeflateRect( +1, +1, +1, +1 );
				pDC->FillSolidRect(rect, RGB(0, 0, 0));
			}
		}
	}

	// cleanup pDC state
	if (pOldPen != NULL)
		pDC->SelectObject(pOldPen);
	if (pOldBrush != NULL)
		pDC->SelectObject(pOldBrush);
	VERIFY(pDC->RestoreDC(-1));
}