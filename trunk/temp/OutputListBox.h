#pragma once


// COutputListBox

namespace UIExt {

class COutputListBox : public CListBox
{
	DECLARE_DYNAMIC(COutputListBox)

protected:
	int	m_nMaxCx;
public:
	enum ItemType { ItemTypeNormal, ItemTypeWarning, ItemTypeError };
	COutputListBox();
	virtual ~COutputListBox();

	int AddString( LPCTSTR lpszItem, ItemType type=ItemTypeNormal );

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

}
