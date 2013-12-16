
// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "ImageViewTest.h"
#include "ChildView.h"

#include <XGraphic/xGraphicObject.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}



void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (cx <= 0 || cy <= 0)
		return;

	for ( int i=0 ; i<2 ; i++ )
	{
		if (m_wndImageView[i].IsMaximized())
			m_wndImageView[i].ShowNormalWindow();
	}

	const int nBorder = 2;
	CRect rcViewArea(0, 0, cx, cy);

	int nViewTotalHeight = rcViewArea.Height() - (nBorder * (2+1));
	int nViewHeight = nViewTotalHeight / 2;
	int nDummyHeight = nViewTotalHeight % 2;
	int nViewWidth = rcViewArea.Width() - nBorder*2;
	CRect rcMaximizeWnd(rcViewArea.left+nBorder, rcViewArea.top+nBorder, rcViewArea.right-nBorder, rcViewArea.bottom-nBorder);
	int nYPos = nBorder+rcViewArea.top;
	m_wndImageView[0].MoveWindow( rcViewArea.left+nBorder, nYPos, nViewWidth, nViewHeight );
	m_wndImageView[0].SetMaximizedWindowPos( rcMaximizeWnd );
	nYPos += nBorder + nViewHeight;
	m_wndImageView[1].MoveWindow( rcViewArea.left+nBorder, nYPos, nViewWidth, nViewHeight );
	m_wndImageView[1].SetMaximizedWindowPos( rcMaximizeWnd );

	m_wndImageView[0].ZoomFit();
	m_wndImageView[1].ZoomFit();
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	DWORD dwBtnTypes = MBT_ZOOM_INOUT|MBT_ZOOM_FIT|MBT_ZOOM_NOT|MBT_MEASURE|MBT_LOAD|MBT_SAVE|MBT_MAXIMIZE;
	COLORBOX clrBox;
	clrBox.CreateObject( RGB(255,0,0), 0, 0, 100, 100 );
	COLORLINE clrLine;
	clrLine.CreateObject( RGB(255,0,0), 0, 0, 100, 100 );
	COLORELLIPSE clrEllipse;
	clrEllipse.CreateObject( RGB(0,255,0), 100, 100, 400, 400 );
	COLORTEXT clrText;
	clrText.CreateObject( RGB(0,0,255), 20, 20, 20 );
	clrText.SetText( _T("AAAA") );
	COLORPOINT clrPoint;
	clrPoint.CreateObject(RGB(255,0,0), 200, 200);
	for ( int i=0 ; i<2 ; i++ )
	{
		m_wndImageView[i].Create( &m_wndImageViewManager, this );
		m_wndImageView[i].ShowDrawElapsedTime(TRUE);
		m_wndImageView[i].SetMiniButtonType(dwBtnTypes, FALSE);
		m_wndImageView[i].SetMiniButtonType(dwBtnTypes, TRUE);
		m_wndImageView[i].SetImageObject( &m_ImageObject[i] );
		m_wndImageView[i].SetAnimateWindow(TRUE);
		//m_wndImageView[i].SetSyncManager(&m_wndImageViewSyncManager);
		//m_wndImageView[i].SetTrackerMode(TRUE);
		m_wndImageView[i].GetGraphicObject().AddDrawObject( &clrBox, 0 );
		m_wndImageView[i].GetGraphicObject().AddDrawLine(clrLine);
		m_wndImageView[i].GetGraphicObject().AddDrawEllipse(clrEllipse);
		m_wndImageView[i].GetGraphicObject().AddDrawPoint(clrPoint);
		m_wndImageView[i].GetGraphicObject().AddDrawText(clrText, 0);
		m_ImageObject[i].LoadFromFile( _T("C:\\Users\\Public\\Pictures\\Sample Pictures\\Lighthouse.jpg") );
		//m_ImageObject[i].LoadFromFile( _T("E:\\CropImage.bmp") );
	}

	return 0;
}


BOOL CChildView::PreTranslateMessage(MSG* pMsg)
{
	/*if (pMsg->message == WM_MOUSEWHEEL)
	{
		CWnd* pWnd = WindowFromPoint( pMsg->pt );
		//if (pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)))
		{
			return m_wndImageView[0].PreTranslateMessage(pMsg);//SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
		}
	}*/

	return CWnd::PreTranslateMessage(pMsg);
}
