// OutputListBox.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "OutputListBox.h"

using namespace UIExt;

// COutputListBox

IMPLEMENT_DYNAMIC(COutputListBox, CListBox)

COutputListBox::COutputListBox()
{
	m_nMaxCx = 0;
}

COutputListBox::~COutputListBox()
{
}


BEGIN_MESSAGE_MAP(COutputListBox, CListBox)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_KILLFOCUS()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// COutputListBox 메시지 처리기입니다.

void COutputListBox::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	CListBox::OnWindowPosChanging(lpwndpos);
}

int COutputListBox::AddString( LPCTSTR lpszItem, ItemType type/*=ItemTypeNormal*/ )
{
	if ( GetCount() > 100 )
	{
		DeleteString( 0 );
	}

	CDC* pDC = GetDC();
	CSize szText = pDC->GetTextExtent(lpszItem);
	ReleaseDC( pDC );

	if ( m_nMaxCx < szText.cx )
	{
		m_nMaxCx = szText.cx;
	}

	int nIdx = CListBox::AddString( lpszItem );

	SetItemData( nIdx, type );
	
	SetHorizontalExtent( m_nMaxCx );

	SetTopIndex( GetCount()-1 );

	return nIdx;
}

void COutputListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	ASSERT(lpMeasureItemStruct->CtlType == ODT_LISTBOX);

	CString strText(_T("MEASURE"));
	GetText(lpMeasureItemStruct->itemID, strText);
	ASSERT(TRUE != strText.IsEmpty());
	CRect rect;
	GetItemRect(lpMeasureItemStruct->itemID, &rect);

	CDC* pDC = GetDC(); 
	lpMeasureItemStruct->itemHeight = pDC->DrawText(strText, -1, rect, 
		DT_WORDBREAK | DT_CALCRECT); 
	ReleaseDC(pDC);
}

void COutputListBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ((int)lpDrawItemStruct->itemID < 0)
		return; 

	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	COLORREF dwOldTextColor = pDC->GetTextColor();
	int nOldBkMode = pDC->GetBkMode();

	ItemType type = (ItemType)lpDrawItemStruct->itemData;
	COLORREF dwTextColor;
	COLORREF dwBodyColor;

	switch (type)
	{
	case ItemTypeWarning:
		dwTextColor = RGB(123, 96, 0);
		dwBodyColor = RGB(255, 239, 185);
		break;
	case ItemTypeError:
		dwTextColor = RGB(170, 0, 26);
		dwBodyColor = RGB(252, 218, 220);
		break;
	default:
		dwTextColor = RGB(80, 80, 80);
		dwBodyColor = RGB(226, 230, 235);
		break;
	}

	if ( !(lpDrawItemStruct->itemState & ODS_SELECTED) && !(lpDrawItemStruct->itemAction & ODA_FOCUS) )
	{
		dwBodyColor = ::GetSysColor(COLOR_WINDOW);
	}

	CBrush brushBody(dwBodyColor);

	pDC->FillRect( &lpDrawItemStruct->rcItem, &brushBody );

	if (lpDrawItemStruct->itemAction & ODA_FOCUS)
	{
		pDC->DrawFocusRect(&lpDrawItemStruct->rcItem);
	}

	pDC->SetBkMode(TRANSPARENT);

	if (lpDrawItemStruct->itemState & ODS_DISABLED)
	{
		pDC->SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		pDC->SetTextColor(dwTextColor);
	}

	CString strText;
	GetText(lpDrawItemStruct->itemID, strText);
	CRect rect = lpDrawItemStruct->rcItem;

	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;
	
	pDC->DrawText(strText, -1, &rect, nFormat | DT_CALCRECT);
	pDC->DrawText(strText, -1, &rect, nFormat);

	pDC->SetTextColor(dwOldTextColor); 
	pDC->SetBkMode(nOldBkMode);
}

void COutputListBox::PreSubclassWindow()
{
	ASSERT( (GetStyle() & LBS_OWNERDRAWVARIABLE) == LBS_OWNERDRAWVARIABLE );
	ASSERT( (GetStyle() & LBS_HASSTRINGS) == LBS_HASSTRINGS );

	CListBox::PreSubclassWindow();
}

void COutputListBox::OnKillFocus(CWnd* pNewWnd)
{
	CListBox::OnKillFocus(pNewWnd);

	SetCurSel(-1);
}

void UIExt::COutputListBox::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
}


BOOL UIExt::COutputListBox::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}
