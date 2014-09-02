/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"
#include "SidebarMenu.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

const int SEPERATOR_HEIGHT		= 5;
const int HIGHLIGHT_HGAP		= 10;
const int HIGHLIGHT_VGAP		= 12;
const int SIDEBAR_WIDTH			= 30; 
const int TEXT_MARGIN			= 5;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSidebarMenu::CSidebarMenu()
{
	m_dwSideBarStartColor   = RGB(128,128,128);
	m_dwSideBarEndColor     = RGB(168,0,26);
	m_dwSideBarTextColor    = RGB(255,255,250);
	m_dwMenuBkColor			= RGB(255,255,250);
	m_dwHighlightColor      = RGB(128,128,128);
	m_dwTextColor           = RGB(128,128,128);
	m_dwHighlightTextColor  = RGB(225,225,225);	

	m_fntCheck.CreateFont( 18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Marlett") );
}

CSidebarMenu::~CSidebarMenu()
{
	m_fntCheck.DeleteObject();
}

void CSidebarMenu::MeasureItem( LPMEASUREITEMSTRUCT lpMIS )
{
	
	LPCTSTR itemData = (LPCTSTR)lpMIS->itemData;
	TEXTMETRIC tm;
	
	CWindowDC dc(NULL);
	
	GetTextMetrics(dc, &tm);

	int nLength = lstrlen(itemData);
	
	if (nLength != 0)	
	{
		lpMIS->itemHeight = tm.tmHeight + HIGHLIGHT_VGAP; 
	}
	else
	{
		lpMIS->itemHeight = SEPERATOR_HEIGHT;
	}
	
	lpMIS->itemWidth = tm.tmAveCharWidth * nLength + TEXT_MARGIN;
}

#include <locale.h>

void CSidebarMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC dc;
	int oldMode;
	int oldTextColor;
	int nCheckOffset = 18;
	COLORREF cr;
	LPCTSTR lpszText = (LPCTSTR) lpDIS->itemData;
	
	CRect rectFull(lpDIS->rcItem); 
	CRect rectText(rectFull.left + SIDEBAR_WIDTH + nCheckOffset,rectFull.top,rectFull.right,rectFull.bottom);
	CRect rectHiLite = rectText;
	
	rectHiLite.left -= nCheckOffset;
	rectHiLite.left -= HIGHLIGHT_HGAP;
	
	dc.Attach(lpDIS->hDC);
	oldMode=dc.SetBkMode(TRANSPARENT);
	
	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		// Paint the color item in the color requested
		cr = m_dwTextColor;
		CBrush br(m_dwMenuBkColor);
		dc.FillRect(&rectFull, &br);
	}
	
	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		// item has been selected - hilite frame
		
		cr= m_dwHighlightTextColor;
		CBrush br(m_dwHighlightColor);
		CBrush *old = dc.SelectObject(&br);
		dc.FillRect(&rectHiLite, &br);
		//dc.RoundRect(&rectHiLite,CPoint(16,16));
		dc.SelectObject(old);
	}
	
	if (!(lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & ODA_SELECT))
	{
		// Item has been de-selected 
		cr = m_dwTextColor;
		CBrush br(m_dwMenuBkColor);
		dc.FillRect(&rectHiLite, &br);
	}
	
	oldTextColor=dc.SetTextColor(cr);
	
	if (lpDIS->itemState & ODS_CHECKED)
	{
		CRect rcCheck = rectText;
		rcCheck.left -= nCheckOffset;
		rcCheck.right = rcCheck.left + nCheckOffset;
		char szCheck[4] = { 0x61, 0x00, 0x00, 0x00 };
		CFont* pOldFont = dc.SelectObject( &m_fntCheck );
		dc.DrawText( (LPCTSTR)szCheck, 1, &rcCheck, DT_SINGLELINE|DT_VCENTER|DT_LEFT );
		dc.SelectObject( pOldFont );
		//dc.SetTextColor( RGB(0xff,0,0) );
	}

	dc.DrawText(
		lpszText,
		(int)_tcslen(lpszText),
		&rectText,
		DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);

	if (lpDIS->itemAction & ODA_DRAWENTIRE)
	{
		CRect rectG ;
		dc.GetClipBox(rectG);

		dc.FillSolidRect( CRect(rectG.left, rectG.top, rectG.left+20, rectG.bottom), m_dwSideBarEndColor );
		
		LOGFONT lf;
		NONCLIENTMETRICS ncm;
		CFont font;
		
		ncm.cbSize = sizeof(ncm);
		//Retrieving the system font settings
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 
			ncm.cbSize, &ncm,0);
		
		memset (&lf, 0, sizeof (LOGFONT));
		
		lf = ncm.lfMenuFont; //storing all default menu font attributes
		lf.lfOrientation = lf.lfEscapement = 900;// 90 degree rotation
		lf.lfWeight = FW_BOLD;
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfHeight = lf.lfHeight * 10 / 9;

		font.CreateFontIndirect ( &lf );
		CFont * OldFont = dc.SelectObject ( &font );
		COLORREF oldColor = dc.SetTextColor(RGB(128,128,128));
		dc.SetTextColor(m_dwSideBarTextColor);
		rectG.bottom += 10;
		dc.DrawText ( m_strTitle, &rectG, DT_BOTTOM | DT_LEFT | DT_SINGLELINE );	
		dc.SetTextColor(oldColor);
		dc.SelectObject(OldFont);
	}
	
	dc.SetBkMode(oldMode);
	dc.SetTextColor(oldTextColor);
	
	//Handling the menu separators if any
	if (lstrlen(lpszText)==0 )
	{
		CBrush b(RGB(255,0,0));
		CPen p;
		
		COLORREF color = RGB(255,255,255); //separator color
		
		color -= m_dwMenuBkColor; // to make it visible on all backgound colors
		//color += RGB(50, 50, 50);
		color = RGB(200,200,200);
		p.CreatePen(PS_SOLID , 1, color);
		CBrush *b1 = dc.SelectObject(&b);
		CPen *p1 = dc.SelectObject(&p);
		
		
		dc.MoveTo(rectHiLite.left+2,rectHiLite.top+2);
		dc.LineTo(rectHiLite.left+rectHiLite.Width(),
			rectHiLite.top+2);
		
		dc.SelectObject(b1);
		dc.SelectObject(p1);
	}
	
	dc.Detach();
}

