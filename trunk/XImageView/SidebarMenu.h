/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#if !defined(AFX_SIDEBARMENU_H__143C7DA6_5556_41BF_A9ED_D67367520658__INCLUDED_)
#define AFX_SIDEBARMENU_H__143C7DA6_5556_41BF_A9ED_D67367520658__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSidebarMenu : public CMenu
{
public:
	CSidebarMenu();
	virtual ~CSidebarMenu();

    void SetSideBarText( LPCTSTR szText ) { m_strTitle = szText; }
	void SetSideBarTextColor( COLORREF dwColor ) { m_dwSideBarTextColor = dwColor; }
    void SetSideBarColor( COLORREF dwStartColor, COLORREF dwEndColor ) { m_dwSideBarStartColor= dwStartColor, m_dwSideBarEndColor = dwEndColor; }
    void SetMenuBkColor( COLORREF dwColor ) { m_dwMenuBkColor = dwColor; }
	void SetHightlightColor( COLORREF dwColor ) { m_dwHighlightColor = dwColor; }
	void SetMenuTextColor( COLORREF dwColor ) { m_dwTextColor = dwColor; }
	void SetTextHightlightColor( COLORREF dwColor ) { m_dwHighlightTextColor = dwColor; }

	virtual void MeasureItem( LPMEASUREITEMSTRUCT lpMeasureItemStruct );
    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

protected:

	CString		m_strTitle;
	COLORREF	m_dwSideBarTextColor;
	COLORREF	m_dwSideBarStartColor;
    COLORREF	m_dwSideBarEndColor;
	COLORREF	m_dwMenuBkColor;
	COLORREF	m_dwHighlightColor;
	COLORREF	m_dwTextColor;
	COLORREF	m_dwHighlightTextColor;

	CFont		m_fntCheck;

};

#endif // !defined(AFX_SIDEBARMENU_H__143C7DA6_5556_41BF_A9ED_D67367520658__INCLUDED_)
