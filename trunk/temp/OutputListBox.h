#pragma once


// COutputListBox

namespace UIExt {

class COutputListBox : public CListBox
{
	DECLARE_DYNAMIC(COutputListBox)

protected:
public:
	COutputListBox();
	virtual ~COutputListBox();

	int AddItem( CMyDialog* pDialog );

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void PreSubclassWindow();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
};

}
