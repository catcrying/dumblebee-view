// OutputListBox.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "OutputListBox.h"

using namespace UIExt;

// COutputListBox

IMPLEMENT_DYNAMIC(COutputListBox, CListBox)

COutputListBox::COutputListBox()
{
}

COutputListBox::~COutputListBox()
{
}


BEGIN_MESSAGE_MAP(COutputListBox, CListBox)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_KILLFOCUS()
END_MESSAGE_MAP()

// COutputListBox 메시지 처리기입니다.

void COutputListBox::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CListBox::OnWindowPosChanging(lpwndpos);
}

int COutputListBox::AddItem( CMyDialog* pDialog )
{
	int nIdx = CListBox::AddString( _T("") );
	
	pDialog->SetParent(this);

	SetItemData( nIdx, pDialog );
	return nIdx;
}

void COutputListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);

	lpMeasureItemStruct->itemHeight = 100;	// 다이얼로그 크기
}

void COutputListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ((int)lpDrawItemStruct->itemID < 0)
		return; 
		
	CRect rcClient = lpDrawItemStruct->rcItem;

	CMyDialog* pDialog = (CMyDialog*)lpDrawItemStruct->itemData;
	pDialog->MoveWindow( rcClient );

}

void COutputListBox::PreSubclassWindow()
{
	ASSERT( (GetStyle() & LBS_OWNERDRAWVARIABLE) == LBS_OWNERDRAWVARIABLE );

	CListBox::PreSubclassWindow();
}

void COutputListBox::OnKillFocus(CWnd* pNewWnd)
{
	CListBox::OnKillFocus(pNewWnd);

	SetCurSel(-1);
}
