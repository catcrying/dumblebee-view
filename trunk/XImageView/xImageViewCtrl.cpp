// xImageViewCtrl.cpp : implementation file
//

#include "stdafx.h"
#include <XImageView/xImageViewCtrl.h>

#include <winuser.h>

#include <XUtil/String/xString.h>
#include <XUtil/xUtils.h>

#include <afxtempl.h>
#include <algorithm>
#include <functional>
#include "GdiplusExt.h"
#include "resource.h"

#include <XImageView/xImageScrollView.h>
#include <XImageView/xImageView.h>

#include <XImage/xImageObject.h>

using namespace Gdiplus;

#define IDC_STATIC_STATUS               1017

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CxImageViewArray
{
public:
	CArray<CxImageViewCtrl*>	m_Views;
public:
	CxImageViewArray() {}
	~CxImageViewArray() {}
};

//////////////////////////////////////////////////////////////////////////
// CxImageViewManager
CxImageViewManager::CxImageViewManager()
{
	m_pViewArray = new CxImageViewArray();
	m_pViewArray->m_Views.RemoveAll();
}

CxImageViewManager::~CxImageViewManager()
{
	delete m_pViewArray;
}

void CxImageViewManager::AddView( CxImageViewCtrl* pView )
{ 
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		if (m_pViewArray->m_Views.GetAt(nI) == pView)
			return;
	}
	m_pViewArray->m_Views.Add( pView );
}

void CxImageViewManager::Reset()
{
	m_pViewArray->m_Views.RemoveAll();
}

void CxImageViewManager::MaximizeWindow( CxImageViewCtrl* pView )
{
	CxImageViewCtrl* pOtherView;
	
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		pOtherView = m_pViewArray->m_Views.GetAt(nI);
		if (pOtherView != pView)
		{
			if (pOtherView->m_bIsMaximized)
			{
				pOtherView->NormalWindow();
				break;
			}
		}
	}

	if (pView->m_bAnimateWindow)
	{
		pView->AnimateWindow(50, AW_HIDE|AW_CENTER);
		pView->RedrawWindow();
	}
	else
	{
		pView->ShowWindow(SW_HIDE);
	}
	pView->BringWindowToTop();
	pView->MaximizeWindow();
	if (pView->m_bAnimateWindow)
	{
		pView->AnimateWindow(100, AW_ACTIVATE|AW_CENTER);
		pView->RedrawWindow();
	}
	else
	{
		pView->ShowWindow(SW_SHOW);
	}
	
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		pOtherView = m_pViewArray->m_Views.GetAt(nI);
		if (pOtherView != pView)
			pOtherView->ShowWindow(SW_HIDE);
	}
}

void CxImageViewManager::NormalWindow( CxImageViewCtrl* pView )
{
	CxImageViewCtrl* pOtherView;
	
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		pOtherView = m_pViewArray->m_Views.GetAt(nI);
		pOtherView->ShowWindow(SW_SHOW);
	}

	//if (pView->m_bAnimateWindow)
	//	pView->AnimateWindow(100, AW_HIDE|AW_CENTER);
	pView->BringWindowToTop();
	pView->NormalWindow();
	//pView->ShowWindow(SW_SHOW);
}

//////////////////////////////////////////////////////////////////////////
// CxImageViewSyncManager
CxImageViewSyncManager::CxImageViewSyncManager()
{ 
	m_bFixZoom = FALSE; 
	m_bMouseTrackPoint = FALSE;
	m_pViewArray = new CxImageViewArray();
}

CxImageViewSyncManager::~CxImageViewSyncManager()
{
	delete m_pViewArray;
}

void CxImageViewSyncManager::AddView( CxImageViewCtrl* pView )
{
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		if (m_pViewArray->m_Views.GetAt(nI) == pView)
			return;
	}
	m_pViewArray->m_Views.Add( pView );
}

void CxImageViewSyncManager::Reset()
{
	m_pViewArray->m_Views.RemoveAll();
}

void CxImageViewSyncManager::SyncDevContext( CxImageViewCtrl* pView, IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage )
{
	CxImageViewCtrl* pOtherView;
	for (INT_PTR nI=0 ; nI<m_pViewArray->m_Views.GetCount() ; nI++)
	{
		pOtherView = m_pViewArray->m_Views.GetAt(nI);
		if (pOtherView != pView)
		{
			pOtherView->OnSyncDevContext( pIDC, ptImage, bUpdateImage );
		}
	}
}

BOOL CxImageViewSyncManager::IsFixZoom()
{
	return m_bFixZoom;
}

void CxImageViewSyncManager::SetFixZoom( BOOL bFix )
{
	m_bFixZoom = bFix;
}

BOOL CxImageViewSyncManager::IsTrackMousePoint()
{
	return m_bMouseTrackPoint;
}
void CxImageViewSyncManager::SetTrackMousePoint( BOOL bTrack )
{
	m_bMouseTrackPoint = bTrack;
}

/////////////////////////////////////////////////////////////////////////////
// CxImageViewCtrl dialog

//CxImageViewManager CxImageViewCtrl::m_ViewManager;
IMPLEMENT_DYNCREATE(CxImageViewCtrl, CWnd)
CxImageViewCtrl::CxImageViewCtrl(CWnd* pParent /*=NULL*/) :
	m_bIsMaximized( FALSE ),
	m_bEnableMoveWindow( FALSE ),
	m_rcMaxWinPos( 0, 0, 0, 0 ),
	m_eOldScreenModeForTracker(ImageViewMode::ScreenModeNone),
	m_pBitmap(NULL), m_bAnimateWindow(TRUE),
	m_fnOnEvent(NULL), m_lpUsrDataOnEvent(NULL),
	m_fnOnDrawExt(NULL), m_lpUsrDataOnDrawExt(NULL),
	m_fnOnMeasure(NULL), m_lpUsrDataOnMeasure(NULL),
	m_fnOnConfirmTracker(NULL), m_lpUsrDataOnConfirmTracker(NULL),
	m_fnOnFireMouseEvent(NULL), m_lpUsrDataOnFirMouseEvent(NULL),
	m_nIndexData(0), m_pRegisterCB(NULL),
	m_bShowTitle(TRUE), m_bShowStatus(TRUE), 
	m_bCreateInnerControl(FALSE),
	m_pViewManager(NULL), m_pSyncManager(NULL),
	m_bSyncDevContext(TRUE),
	m_rcStatus(0,0,0,0), m_rcTitle(0,0,0,0),
	m_pImageObject(NULL)
{
	m_bIsModal = FALSE;
	m_bInActive = TRUE;

	m_pImageView = NULL;
	m_nMiniBtnIconSize = 0;
	m_nMiniBtnIconCount = 0;

	m_bIsButtonDown = FALSE;

	m_bShowMiniButtons = TRUE;

	m_pMiniBtnIconBlack = m_pMiniBtnIconWhite = m_pMiniBtnIconColor;
	m_pMiniBtnIconBlack = ::GdipLoadImageFromRes( GetResourceHandle(), _T("PNG"), PNG_MINIBTN_ICON_BLACK );
	m_pMiniBtnIconWhite = ::GdipLoadImageFromRes( GetResourceHandle(), _T("PNG"), PNG_MINIBTN_ICON_WHITE );
	m_pMiniBtnIconColor = ::GdipLoadImageFromRes( GetResourceHandle(), _T("PNG"), PNG_MINIBTN_ICON_COLOR );

	m_eMiniButtonColorType = MiniButtonColorTypeWhite;
	m_pMiniBtnIcon = m_pMiniBtnIconWhite;

	if (m_pMiniBtnIconBlack)
	{
		m_nMiniBtnIconSize = (int)m_pMiniBtnIconBlack->GetHeight();
		m_nMiniBtnIconCount = (int)m_pMiniBtnIconBlack->GetWidth() / m_nMiniBtnIconSize;
	}

	m_strTitle = _T("IMAGE VIEW");
	m_bShowTitleIcon = TRUE;

	//m_dwTitleBodyColor = RGB(34, 34, 34);
	//m_dwStatusBodyColor = RGB(34, 34, 34);
	m_dwTitleBodyColor = RGB(74, 98, 117);//RGB(119, 107, 95);//RGB(64, 85, 121);
	m_dwStatusBodyColor = RGB(210, 210, 210);//RGB(35, 45, 59);//RGB(100, 88, 76);//RGB(52, 71, 104);

	m_dwStatusTextColor = RGB(109, 110, 118);

	m_dwBodyColor = RGB(220, 220, 240);
	m_dwBorderColors[0] = (DWORD)-1;
	m_dwBorderColors[1] = (DWORD)-1;
	m_dwBorderColors[2] = (DWORD)-1;
	m_BodyBrush.CreateSolidBrush( m_dwBodyColor );

	m_nTitleBarHeight = 30;

	m_dwActiveTitleColor = RGB(230, 230, 230);//RGB(255, 255, 255);
	m_dwInactiveTitleColor = RGB(230, 230, 230);

	m_dwButtonHoverColor = RGB(255, 255, 255);
	m_dwButtonPressColor = RGB(200, 30, 30);
	m_dwButtonBorderHoverColor = RGB(255, 255, 255);
	m_dwButtonBorderPressColor = RGB(200, 30, 30);
	
	m_TitleFont.CreateFont( 14, 0, 0, 0,
		FW_BOLD, FALSE, FALSE, 0, 
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial") );

	m_dwMiniButtonType[0] = MBT_BUTTON_ALL;
	m_dwMiniButtonType[1] = MBT_BUTTON_ALL;

	m_eZoomCheckedIndex = IndexNone;
	m_eButtonHoverIndex = IndexNone;
	m_eButtonPressIndex = IndexNone;
	m_eButtonFirst = IndexNone;
	m_eButtonLast = IndexNone;

	m_nIndexViewWidth = 140;
	m_nIndexViewHeight = 140;

	m_nChannel = 1;
	m_nPixelDepth = 8;
	m_nPixelLevel = 0;

	NONCLIENTMETRICS ncm = { sizeof(NONCLIENTMETRICS) };
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

	USES_CONVERSION;

	WCHAR* wszFaceName;
	CString strX = ncm.lfMessageFont.lfFaceName;
	if ( strX.IsEmpty() )
		wszFaceName = L"Arial";
	else
		wszFaceName = (WCHAR*)T2W(ncm.lfMessageFont.lfFaceName);
	m_pFontFamily = ::new FontFamily( wszFaceName );

	m_strTitle.LoadString(GetResourceHandle(), IDS_IMAGE_VIEW);

	m_pImageObject = new CxImageObject();
}

CxImageViewCtrl::~CxImageViewCtrl()
{
	if (m_pMiniBtnIconBlack)
		delete m_pMiniBtnIconBlack;
	if (m_pMiniBtnIconWhite)
		delete m_pMiniBtnIconWhite;
	if (m_pMiniBtnIconColor)
		delete m_pMiniBtnIconColor;
	if (m_pFontFamily)
		::delete m_pFontFamily;
	if (m_pImageObject)
		delete m_pImageObject;
}

void CxImageViewCtrl::SetEnableMoveWindow( BOOL bEnable ) 
{ 
	m_bEnableMoveWindow = bEnable; 
}

void CxImageViewCtrl::SetMaximizedWindowPos( RECT rcWindow )
{ 
	m_rcMaxWinPos = rcWindow;
	m_bIsMaximized = FALSE; 
}

BOOL CxImageViewCtrl::SetAnimateWindow( BOOL bEnable ) 
{
	BOOL bOldState = m_bAnimateWindow;
	m_bAnimateWindow = bEnable;
	return bOldState;
}

void CxImageViewCtrl::SetOnDrawExt( FnOnDrawExt _fnOnDrawExt, LPVOID _lpUsrData ) 
{
	m_fnOnDrawExt = _fnOnDrawExt; m_lpUsrDataOnDrawExt = _lpUsrData;		
}
void CxImageViewCtrl::SetOnMeasure( FnOnMeasure _fnOnMeasure, LPVOID _lpUsrData ) 
{
	m_fnOnMeasure = _fnOnMeasure; m_lpUsrDataOnMeasure = _lpUsrData;
	if ( m_pImageView )
		m_pImageView->SetOnMeasure( m_fnOnMeasure, m_lpUsrDataOnMeasure );
}
void CxImageViewCtrl::SetOnEvent( FnOnEvent _fnOnEvent, LPVOID _lpUsrData )
{
	m_fnOnEvent = _fnOnEvent; m_lpUsrDataOnEvent = _lpUsrData;
	if ( m_pImageView )
		m_pImageView->SetOnEvent( m_fnOnEvent, m_lpUsrDataOnEvent );
}
void CxImageViewCtrl::SetOnConfirmTracker( FnOnConfirmTracker _fnOnConfirmTracker, LPVOID _lpUsrData ) 
{
	m_fnOnConfirmTracker = _fnOnConfirmTracker; m_lpUsrDataOnConfirmTracker = _lpUsrData;
	if ( m_pImageView )
		m_pImageView->SetOnConfirmTracker( m_fnOnConfirmTracker, m_lpUsrDataOnConfirmTracker );
}	
void CxImageViewCtrl::SetOnFireMouseEvent( FnOnFireMouseEvent _fnOnFireMouseEvent, LPVOID _lpUsrData ) 
{
	m_fnOnFireMouseEvent = _fnOnFireMouseEvent; m_lpUsrDataOnFirMouseEvent = _lpUsrData;
	if ( m_pImageView )
		m_pImageView->SetOnFireMouseEvent( m_fnOnFireMouseEvent, m_lpUsrDataOnFirMouseEvent );
}

void CxImageViewCtrl::SetRealSizePerPixel( float fRealPixelSize ) { m_pImageView->SetRealSizePerPixel(fRealPixelSize); }
float CxImageViewCtrl::GetRealSizePerPixel() const { return m_pImageView->GetRealSizePerPixel(); }
void CxImageViewCtrl::SetRealSizePerPixelW( float fRealPixelSizeW ) const { m_pImageView->SetRealSizePerPixelW(fRealPixelSizeW); }
float CxImageViewCtrl::GetRealSizePerPixelW() const { return m_pImageView->GetRealSizePerPixelW(); }
void CxImageViewCtrl::SetRealSizePerPixelH( float fRealPixelSizeH ) { m_pImageView->SetRealSizePerPixelH(fRealPixelSizeH); }
float CxImageViewCtrl::GetRealSizePerPixelH() const { return m_pImageView->GetRealSizePerPixelH(); }

CxImageObject* CxImageViewCtrl::GetVisibleImageObject() { return m_pImageView!=NULL ? m_pImageView->GetImageObject() : NULL; }
CxImageObject* CxImageViewCtrl::GetImageObject() { return m_pImageObject; }

CxGraphicObject& CxImageViewCtrl::GetGraphicObject() { return m_pImageView->GetGraphicObject(); }
IxDeviceContext* CxImageViewCtrl::GetIDeviceContext() { return m_pImageView; }

void CxImageViewCtrl::AttachGraphicObject( CxGraphicObject* pGO ) { m_pImageView->AttachGraphicObject(pGO); }
CxGraphicObject* CxImageViewCtrl::DetachGraphicObject() { return m_pImageView->DetachGraphicObject(); }
BOOL CxImageViewCtrl::IsGraphicObjectAttached() { return m_pImageView->IsGraphicObjectAttached(); }

void CxImageViewCtrl::SetTrackerPosition( CRect& rcTrack ) { m_pImageView->SetTracker( rcTrack ); }
BOOL CxImageViewCtrl::IsTrackerMode() { return m_pImageView->GetScreenMode() == ImageViewMode::ScreenModeTracker ? TRUE : FALSE; }

void CxImageViewCtrl::SetMouseWheelMode( ImageViewMode::MouseWheelMode eMouseWheelMode ) { m_pImageView->SetMouseWheelMode( eMouseWheelMode ); }
ImageViewMode::MouseWheelMode CxImageViewCtrl::GetMouseWheelMode() const { return m_pImageView->GetMouseWheelMode(); }

void CxImageViewCtrl::SetTrackerMode( BOOL bSet, BOOL bUseFixedTracker/*=FALSE*/ ) 
{ 
	if ( bSet )
	{
		if ( IsTrackerMode() )
			return;
		m_pImageView->SetTrackMode( bUseFixedTracker );
		m_eOldScreenModeForTracker = m_pImageView->SetScreenMode( ImageViewMode::ScreenModeTracker ); 
	}
	else
	{
		if ( !IsTrackerMode() )
			return;
		m_pImageView->SetScreenMode( m_eOldScreenModeForTracker );
	}
}

void CxImageViewCtrl::SetTitle( LPCTSTR lpszTitle, BOOL bShowIcon/*=TRUE*/ )
{
	m_strTitle = lpszTitle;
	m_bShowTitleIcon = bShowIcon;
	RedrawTitle();
}

void CxImageViewCtrl::SetTitleBarHeight( int nHeight )
{
	if (nHeight < 18)
		nHeight = 18;
	m_nTitleBarHeight = nHeight;
	if ( !::IsWindow( GetSafeHwnd() ) ) return;
	CRect rc;
	GetClientRect(rc);
	ArrangeInnerView( rc.Width(), rc.Height() );
}

void CxImageViewCtrl::SetMiniButtonType( DWORD dwMBType, BOOL bMaximize ) 
{ 
	m_dwMiniButtonType[bMaximize ? 1 : 0] = dwMBType;
	if ( !::IsWindow( GetSafeHwnd() ) ) return;

	InvalidateRect( m_rcMiniButton );
}

void CxImageViewCtrl::SetMiniButtonTypeAll( DWORD dwMBType )
{
	m_dwMiniButtonType[0] = m_dwMiniButtonType[1] = dwMBType;
	if ( !::IsWindow( GetSafeHwnd() ) ) return;

	InvalidateRect( m_rcMiniButton );
}

void CxImageViewCtrl::ModifyMiniButtonType( DWORD dwRemoveType, DWORD dwAddType, BOOL bMaximize )
{
	m_dwMiniButtonType[bMaximize ? 1 : 0] &= ~dwRemoveType;
	m_dwMiniButtonType[bMaximize ? 1 : 0] |= dwAddType;

	if ( !::IsWindow( GetSafeHwnd() ) ) return;

	InvalidateRect( m_rcMiniButton );
}

void CxImageViewCtrl::ModifyMiniButtonTypeAll( DWORD dwRemoveType, DWORD dwAddType )
{
	m_dwMiniButtonType[0] &= ~dwRemoveType;
	m_dwMiniButtonType[0] |= dwAddType;
	m_dwMiniButtonType[1] &= ~dwRemoveType;
	m_dwMiniButtonType[1] |= dwAddType;

	if ( !::IsWindow( GetSafeHwnd() ) ) return;

	InvalidateRect( m_rcMiniButton );
}

void CxImageViewCtrl::ImageUpdate()
{
	if ( m_pImageView )
	{
		m_pImageView->Invalidate();
	}
}

void CxImageViewCtrl::ZoomFit( BOOL bCalcScrollBar /*=TRUE*/ )
{
	if ( m_pImageView )
		m_pImageView->ZoomFit( bCalcScrollBar );
}

void CxImageViewCtrl::ZoomTo( CPoint ptImageCenter, float fZoom, BOOL bSyncControl/*=FALSE*/ )
{
	if ( m_pImageView )
		m_pImageView->ImageZoomTo( ptImageCenter, fZoom, bSyncControl );
}

CxImageViewManager* CxImageViewCtrl::GetImageViewManager()
{
	return m_pViewManager;
}

void CxImageViewCtrl::SetImageViewManager( CxImageViewManager* pImageViewManager )
{
	ASSERT( m_pViewManager );
	if ( !m_pViewManager ) return;
	m_pViewManager = pImageViewManager;
	m_pViewManager->AddView( this );
}

CxImageViewSyncManager* CxImageViewCtrl::GetSyncManager()
{
	return m_pSyncManager;
}

void CxImageViewCtrl::SetSyncManager( CxImageViewSyncManager* pSyncManager )
{
	ASSERT( pSyncManager );
	if ( !pSyncManager ) return;
	m_pSyncManager = pSyncManager;
	m_pSyncManager->AddView( this );
}

BOOL CxImageViewCtrl::Create( CxImageViewManager* pViewManager, CWnd* pParentWnd /*= NULL*/ ) 
{ 
	m_pViewManager = pViewManager;
	if ( m_pViewManager )
		m_pViewManager->AddView( this ); 

	if ( pParentWnd )
		pParentWnd->ModifyStyle( 0, WS_CLIPCHILDREN );

	return CWnd::Create( NULL, _T("ImageView"), WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN, CRect(0,0,0,0), pParentWnd, -1 );
}

void CxImageViewCtrl::NormalWindow()
{
	SetWindowPos( NULL, m_rcNormalWinPos.left, m_rcNormalWinPos.top, m_rcNormalWinPos.Width(), m_rcNormalWinPos.Height(), 0 );

	m_bIsMaximized = FALSE;

	ZoomFit();

	if ( m_fnOnEvent )
		(*m_fnOnEvent)( ImageViewEvent::ActionEventNormalScreen, m_nIndexData, m_lpUsrDataOnEvent );
}

void CxImageViewCtrl::MoveWindow( int x, int y, int cx, int cy, BOOL bRepaint/*=TRUE*/, BOOL bZoomFit/*=TRUE*/ )
{
	BOOL bShowScrollBar = m_pImageView->IsShowScrollBar();
	if (bShowScrollBar)
		m_pImageView->ShowScrollBar(FALSE);
	CWnd::MoveWindow(x, y, cx, cy, bRepaint);
	ZoomFit();
	if (bShowScrollBar)
		m_pImageView->ShowScrollBar(TRUE);
}

void CxImageViewCtrl::MaximizeWindow()
{
	if ( m_rcMaxWinPos.Width() <= 0 ) return;

	int nSWW, nSWH;
	m_pImageView->GetScrollBarDimension( nSWW, nSWH );
	BOOL bCalcScrollBar = FALSE;
	if ( nSWW != 0 || nSWH != 0 )
	{
		bCalcScrollBar = TRUE;
	}

	CRect rcWindow;
	GetWindowRect( &rcWindow );
	GetOwner()->ScreenToClient( &rcWindow );
	m_rcNormalWinPos = rcWindow;

	CRect rcMaxWinPos = m_rcMaxWinPos;

	SetWindowPos( NULL, rcMaxWinPos.left, rcMaxWinPos.top, rcMaxWinPos.Width(), rcMaxWinPos.Height(), 0 );

	m_bIsMaximized = TRUE;

	ZoomFit( bCalcScrollBar );

	if ( m_fnOnEvent )
		(*m_fnOnEvent)( ImageViewEvent::ActionEventMaximizedScreen, m_nIndexData, m_lpUsrDataOnEvent );

	return;
}

BEGIN_MESSAGE_MAP(CxImageViewCtrl, CWnd)
	//{{AFX_MSG_MAP(CxImageViewCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_GETMINMAXINFO()
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CxImageViewCtrl message handlers

int CxImageViewCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}
	
	return 0;
}

void CxImageViewCtrl::CreateInnerCtrls()
{
	DWORD dwStyle = GetStyle();
	if ( !(dwStyle & WS_CLIPCHILDREN) )
		ModifyStyle( 0, WS_CLIPCHILDREN );
	if ( !(dwStyle & WS_CLIPSIBLINGS) )
		ModifyStyle( 0, WS_CLIPSIBLINGS );

	TRACE( _T("CxImageViewCtrl::CreateInnerControls: %d\r\n"), m_bCreateInnerControl );
	m_bCreateInnerControl = TRUE;

	InitControl();
}

void CxImageViewCtrl::SetImageBuffer( int nWidth, int nHeight, LPVOID lpBuf )
{
	BOOL bZoomFit = FALSE;
	if ( m_pImageObject->GetWidth() != nWidth || m_pImageObject->GetHeight() != nHeight )
	{
		bZoomFit = TRUE;
	}
	m_pImageObject->CreateFromBuffer( lpBuf, nWidth, nHeight, 8, 1 );

	SetImageObject( m_pImageObject, bZoomFit );
}

void CxImageViewCtrl::SetImageObject( CxImageObject* pImageObject, BOOL bZoomFit )
{
	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}

	ASSERT( m_pImageView );

	m_pImageView->SetImageObject( pImageObject );

	if ( bZoomFit )
	{
		m_pImageView->ZoomFit();
	}
	else
	{
		m_pImageView->RecalcZoomRatio();
	}

	Invalidate();
}

/*
void CxImageViewCtrl::SetIndexViewImageObject( CxImageObject* pImageObject )
{
	CRect rcClient;
	GetClientRect( &rcClient );
	int cx = rcClient.Width();
	int cy = rcClient.Height();

	float fTan;
	if ( pImageObject )
		fTan = (float)pImageObject->GetHeight() / pImageObject->GetWidth();
	else
		fTan = 1.f;
	m_nIndexViewHeight = (int)(m_nIndexViewWidth * fTan);

	int nMaxHeight = (int)(m_nIndexViewWidth*1.5f);
	int nMinHeight = (int)(m_nIndexViewWidth*.9f);
	if ( m_nIndexViewHeight > nMaxHeight )
	{
		m_nIndexViewHeight = nMaxHeight;
	}
	if ( m_nIndexViewHeight < nMinHeight )
	{
		m_nIndexViewHeight = nMinHeight;
	}

	int nSWW, nSWH;
	m_pImageView->GetScrollBarDimension( nSWW, nSWH );

	CRect rcIndexClient( cx-m_nIndexViewWidth-2-nSWW, m_nTitleBarHeight+2, cx-2-nSWW, m_nTitleBarHeight+2+m_nIndexViewHeight );
	ClientToScreen( &rcIndexClient );

	if ( m_wndIndexViewCtrl )
		m_wndIndexViewCtrl.MoveWindow( &rcIndexClient, TRUE );

	m_wndIndexViewCtrl.SetImageObject( pImageObject );

}
*/

void CxImageViewCtrl::OnDestroy() 
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap=NULL;
	}
	CWnd::OnDestroy();
}

BOOL CxImageViewCtrl::InitControl() 
{
	if ( !m_pImageView )
#ifdef USE_NEW_VIEW
		m_pImageView = (CxImageView*)CreateProtectedView( RUNTIME_CLASS(CxImageView), this, NULL, NULL );
#else
		m_pImageView = (CxImageScrollView*)CreateProtectedView( RUNTIME_CLASS(CxImageScrollView), this, NULL, NULL );
#endif
	if ( !m_pImageView && !m_pImageView->GetSafeHwnd() )
		return FALSE;

	m_pImageView->SetBackgroundColor( m_dwBodyColor );
	m_pImageView->SetPopupMenuColor( m_dwTitleBodyColor, m_dwActiveTitleColor );

	if ( m_pRegisterCB == NULL )
	{
		m_pImageView->SetOnDrawExt( _OnDrawExt, this );

		m_pImageView->SetOnEvent( m_fnOnEvent, m_lpUsrDataOnEvent );
		m_pImageView->SetOnFireMouseEvent( m_fnOnFireMouseEvent, m_lpUsrDataOnFirMouseEvent );
		m_pImageView->SetOnMeasure( m_fnOnMeasure, m_lpUsrDataOnMeasure );
		m_pImageView->SetOnConfirmTracker( m_fnOnConfirmTracker, m_lpUsrDataOnConfirmTracker );
	}
	else
	{
		m_fnOnDrawExt				= m_pRegisterCB->fnOnDrawExt;
		m_fnOnMeasure				= m_pRegisterCB->fnOnMeasure;
		m_fnOnConfirmTracker		= m_pRegisterCB->fnOnConfirmTracker;
		
		m_fnOnFireMouseEvent		= m_pRegisterCB->fnOnFireMouseEvent;
		m_fnOnEvent					= m_pRegisterCB->fnOnEvent;
		
		m_lpUsrDataOnDrawExt		= m_pRegisterCB->lpUsrData[0];
		m_lpUsrDataOnMeasure		= m_pRegisterCB->lpUsrData[1];
		m_lpUsrDataOnConfirmTracker	= m_pRegisterCB->lpUsrData[2];
		
		m_lpUsrDataOnFirMouseEvent	= m_pRegisterCB->lpUsrData[3];
		m_lpUsrDataOnEvent			= m_pRegisterCB->lpUsrData[4];
		
		m_pImageView->SetRegisterCallBack( m_nIndexData, m_pRegisterCB );
		m_pImageView->SetOnDrawExt( _OnDrawExt, this );
	}

	CRect rcClient;
	GetClientRect(rcClient);
	ArrangeInnerView( rcClient.Width(), rcClient.Height() );

	if ( m_MemDC.GetSafeHdc() == NULL )
	{
		CClientDC dc(this);
		m_MemDC.CreateCompatibleDC(&dc);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CxImageViewCtrl::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CWnd::OnShowWindow(bShow, nStatus);
}

BOOL CxImageViewCtrl::OnEraseBkgnd(CDC* pDC) 
{
	if ( !m_pBitmap ) return FALSE;
	if ( m_MemDC.GetSafeHdc() == NULL )
	{
		m_MemDC.CreateCompatibleDC(pDC);
	}
	CBitmap * pOldBitmap = m_MemDC.SelectObject(m_pBitmap);
	CRect rect;
	GetClientRect(rect);
	m_MemDC.FillSolidRect(rect, m_dwBodyColor);
	m_MemDC.SelectObject(pOldBitmap);
	return TRUE;
}

void CxImageViewCtrl::OnBtnLoad() 
{
	m_eButtonHoverIndex = IndexNone;

	if ( m_fnOnEvent )
	{
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventLoadClick, m_nIndexData, m_lpUsrDataOnEvent ) ) 
		{
			return;
		}
	}

	CString strFilter;
	strFilter = _T("Bitmap Files(*.bmp)|*.bmp|All Files(*.*)|*.*|");
	CFileDialog FileDialog( TRUE, _T("*.bmp"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, strFilter );
	
	if ( FileDialog.DoModal() == IDOK )
	{
		if ( !m_pImageObject->LoadFromFile( FileDialog.GetPathName() ) )
		{
			CString strMsg;
			strMsg.LoadString(GetResourceHandle(), IDS_CANNOT_LOAD_IMAGE);
			CString strError;
			strError.LoadString(GetResourceHandle(), IDS_ERROR );
			::MessageBox( GetSafeHwnd(), 
				strMsg, 
				strError, MB_OK|MB_ICONSTOP );
			return;
		}

		m_pImageView->SetImageObject( m_pImageObject );

		m_pImageView->ZoomFit();
		m_pImageView->Invalidate();

		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventLoadFile, m_nIndexData, m_lpUsrDataOnEvent );

	}
}

void CxImageViewCtrl::OnBtnSave() 
{
	m_eButtonHoverIndex = IndexNone;

	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventSaveClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	CString strFilter;
	strFilter = _T("Bitmap Files(*.bmp)|*.bmp|All Files(*.*)|*.*|");
	CFileDialog FileDialog( FALSE, _T("*.bmp"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, strFilter );
	
	if ( FileDialog.DoModal() == IDOK )
	{
		if ( GetVisibleImageObject() )
		{
			if ( !GetVisibleImageObject()->SaveToFile( FileDialog.GetPathName() ) )
			{
				CString strMsg;
				strMsg.LoadString(GetResourceHandle(), IDS_CANNOT_SAVE_IMAGE);
				CString strError;
				strError.LoadString(GetResourceHandle(), IDS_ERROR );
				::MessageBox( GetSafeHwnd(), 
					strMsg, 
					strError, MB_OK|MB_ICONSTOP );
				return;
			}			
		}

		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventSaveFile, m_nIndexData, m_lpUsrDataOnEvent );
	}
}

BOOL CxImageViewCtrl::SaveImage( LPCTSTR lpszPathName )
{
	CWaitCursor wait;
	if ( GetVisibleImageObject() )
	{
		if ( !GetVisibleImageObject()->SaveToFile( lpszPathName ) )
		{
			return FALSE;
		}
		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventSaveFile, m_nIndexData, m_lpUsrDataOnEvent );
		return TRUE;
	}
	return FALSE;
}

BOOL CxImageViewCtrl::LoadImage( LPCTSTR lpszPathName )
{
	CWaitCursor wait;
	if ( !m_pImageObject->LoadFromFile( lpszPathName ) )
	{
		return FALSE;
	}
	
	m_pImageView->SetImageObject( m_pImageObject );
	m_pImageView->ZoomFit();
	m_pImageView->Invalidate();

	if ( m_fnOnEvent )
		(*m_fnOnEvent)( ImageViewEvent::ActionEventLoadFile, m_nIndexData, m_lpUsrDataOnEvent );

	return TRUE;
}

void CxImageViewCtrl::ShowMaximizedWindow()
{
	if ( m_bIsMaximized ) return;

	if ( !m_bIsModal )
	{	
		if ( m_pViewManager )
			m_pViewManager->MaximizeWindow( this );
		else
			MaximizeWindow();
	}
	else
	{
		MaximizeWindow();
	}
}

void CxImageViewCtrl::ShowNormalWindow()
{
	if ( !m_bIsMaximized ) return;

	if ( !m_bIsModal )
	{	
		if ( m_pViewManager )
			m_pViewManager->NormalWindow( this );
		else
			NormalWindow();
	}
	else
	{
		NormalWindow();
	}	
}

void CxImageViewCtrl::OnBtnMaximize() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventMaximizeClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;
		
	if ( !m_bIsModal )
	{
		if ( m_pViewManager )
		{
			if ( m_bIsMaximized )
			{
				m_pViewManager->NormalWindow( this );
			}
			else
			{
				m_pViewManager->MaximizeWindow( this );
			}
		}
		else
		{
			if ( m_bIsMaximized )
			{
				NormalWindow();
			}
			else
			{
				MaximizeWindow();
			}
		}
	}
	else
	{
		if ( m_bIsMaximized )
		{
			NormalWindow();
		}
		else
		{
			MaximizeWindow();
		}
	}	
}

void CxImageViewCtrl::OnBtnZoomFit() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventZoomFitClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	m_pImageView->ZoomFit();
}

void CxImageViewCtrl::OnBtnZoomNot() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventZoomNotClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	m_pImageView->ZoomNot();
}

void CxImageViewCtrl::OnBtnZoomInout() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventZoomInOutClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	if (m_eZoomCheckedIndex == IndexZoomInOut)
		m_eZoomCheckedIndex = IndexNone;
	else
		m_eZoomCheckedIndex = IndexZoomInOut;
	m_pImageView->SetScreenMode( m_eZoomCheckedIndex == IndexZoomInOut ? ImageViewMode::ScreenModeZoomInOut : ImageViewMode::ScreenModePanning );
}

void CxImageViewCtrl::OnBtnZoomIn() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventZoomInClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	if (m_eZoomCheckedIndex == IndexZoomIn)
		m_eZoomCheckedIndex = IndexNone;
	else
		m_eZoomCheckedIndex = IndexZoomIn;
	m_pImageView->SetScreenMode( m_eZoomCheckedIndex == IndexZoomIn ? ImageViewMode::ScreenModeZoomIn : ImageViewMode::ScreenModePanning );
}

void CxImageViewCtrl::OnBtnZoomOut() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventZoomOutClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	if (m_eZoomCheckedIndex == IndexZoomOut)
		m_eZoomCheckedIndex = IndexNone;
	else
		m_eZoomCheckedIndex = IndexZoomOut;
	m_pImageView->SetScreenMode( m_eZoomCheckedIndex == IndexZoomOut ? ImageViewMode::ScreenModeZoomOut : ImageViewMode::ScreenModePanning );
}

void CxImageViewCtrl::OnBtnMeasure() 
{
	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ButtonEventMeasureClick, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	if (m_eZoomCheckedIndex == IndexMeasure)
		m_eZoomCheckedIndex = IndexNone;
	else
		m_eZoomCheckedIndex = IndexMeasure;
	if ( ImageViewMode::ScreenModeMeasure == m_pImageView->SetScreenMode( m_eZoomCheckedIndex == IndexMeasure ? ImageViewMode::ScreenModeMeasure : ImageViewMode::ScreenModePanning ) )
	{
		ImageUpdate();
	}
}

void CxImageViewCtrl::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);

	if ( cx <= 0 || cy <= 0 ) return;

	if ( m_pBitmap != NULL ) delete m_pBitmap;
	m_pBitmap = new CBitmap;
	ASSERT( m_pBitmap );
	CClientDC dc(this);
	VERIFY( m_pBitmap->CreateCompatibleBitmap(&dc, cx, cy) );

	ArrangeInnerView( cx, cy );

	int nSWW, nSWH;
	m_pImageView->GetScrollBarDimension( nSWW, nSWH );

	Invalidate( TRUE );
}

void CxImageViewCtrl::ShowTitleWindow( BOOL bShow )
{
	m_bShowTitle = bShow;
	if ( !m_bCreateInnerControl )
		return;

	if ( m_bShowTitle )
	{
		ShowAllMiniButtons();
	}
	else
	{
		HideAllMiniButtons();
	}

	CRect rcClient;
	GetClientRect( &rcClient );
	ArrangeInnerView( rcClient.Width(), rcClient.Height() );
}

void CxImageViewCtrl::ShowStatusWindow( BOOL bShow )
{
	m_bShowStatus = bShow;

	if ( !m_bCreateInnerControl )
		return;

	CRect rcClient;
	GetClientRect( &rcClient );
	ArrangeInnerView( rcClient.Width(), rcClient.Height() );
}

void CxImageViewCtrl::ShowScaleBar( BOOL bShow )
{
	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}

	m_pImageView->ShowScaleBar( bShow );
}

void CxImageViewCtrl::ShowDrawElapsedTime( BOOL bShow )
{
	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}

	m_pImageView->ShowDrawElapsedTime( bShow );
}

void CxImageViewCtrl::UseAutoFocus( BOOL bUse )
{
	m_pImageView->UseAutoFocus(bUse);
}

void CxImageViewCtrl::EnableMouseControl( BOOL bEnable )
{
	m_pImageView->EnableMouseControl(bEnable);
}

void CxImageViewCtrl::ShowScrollBar( BOOL bShow )
{
	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}

	m_pImageView->ShowScrollBar( bShow );
}

void CxImageViewCtrl::EnableSyncDevContext( BOOL bSyncDevContext )
{
	m_bSyncDevContext = bSyncDevContext;
}

BOOL CxImageViewCtrl::IsMaximized()
{
	return m_bIsMaximized;
}

void CxImageViewCtrl::ShowDigitize( BOOL bShow )
{
	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
	}

	m_pImageView->ShowDigitize( bShow );
}

void CxImageViewCtrl::HideAllMiniButtons()
{
	m_bShowMiniButtons = FALSE;
	InvalidateRect(m_rcMiniButton);
}

void CxImageViewCtrl::ShowAllMiniButtons()
{
	m_bShowMiniButtons = TRUE;
	InvalidateRect(m_rcMiniButton);
}

void CxImageViewCtrl::ArrangeInnerView( int cx, int cy )
{
	if ( !m_pImageView ) return;
	if ( !::IsWindow(m_pImageView->GetSafeHwnd()) ) return;
	if ( cx == 0 || cy == 0 ) return;

	m_rcTitle = CRect(0, 0, cx, m_nTitleBarHeight);

#define STATUSBAR_HEIGHT 20
	int nBtmOffset = STATUSBAR_HEIGHT;
	int nTopOffset = m_nTitleBarHeight;

	if ( !m_bShowStatus ) nBtmOffset = 0;
	if ( !m_bShowTitle ) nTopOffset = 0;

	int nBorderOffset = 0;
	if (m_dwBorderColors[0] != (DWORD)-1)
		nBorderOffset++;

	m_pImageView->MoveWindow( nBorderOffset, nTopOffset, cx-nBorderOffset*2, cy-nTopOffset-nBtmOffset );

	if ( m_bShowStatus )
	{
		m_rcStatus = CRect( -1, cy-nBtmOffset-1, cx+1, cy+1 );
	}
}

void CxImageViewCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ( ::GetCapture() != m_hWnd ) ::SetCapture( m_hWnd );

	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ActionEventTitleLButtonDown, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	for ( int i=0 ; i<sizeof(m_rectMiniBtnBody) / sizeof(Rect) ; i++ )
	{
		if ( m_rectMiniBtnBody[i].Contains(point.x, point.y) )
		{
			m_eButtonPressIndex = (ButtonIconIndex)i;
			InvalidateRect( m_rcMiniButton );
			break;
		}
	}

	m_bIsButtonDown = TRUE;

	m_ptLastMouse = point;

	SetFocus();
	
	CWnd::OnLButtonDown(nFlags, point);
}

void CxImageViewCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( ::GetCapture() == m_hWnd )
		::ReleaseCapture();

	if ( m_fnOnEvent )
		if ( (*m_fnOnEvent)( ImageViewEvent::ActionEventTitleLButtonUp, m_nIndexData, m_lpUsrDataOnEvent ) ) return;

	ButtonIconIndex eReleaseIndex = IndexNone;
	for ( int i=0 ; i<sizeof(m_rectMiniBtnBody) / sizeof(Rect) ; i++ )
	{
		if ( m_rectMiniBtnBody[i].Contains(point.x, point.y) )
		{
			eReleaseIndex = (ButtonIconIndex)i;
			break;
		}
	}

	if (m_eButtonPressIndex == eReleaseIndex)
	{
		switch (eReleaseIndex)
		{
		case IndexLoad:
			OnBtnLoad();
			break;
		case IndexSave:
			OnBtnSave();
			break;
		case IndexZoomFit:
			OnBtnZoomFit();
			break;
		case IndexZoomIn:
			OnBtnZoomIn();
			break;
		case IndexZoomOut:
			OnBtnZoomOut();
			break;
		case IndexShowMaximize:
		case IndexShowNormal:
			OnBtnMaximize();
			break;
		case IndexZoomNot:
			OnBtnZoomNot();
			break;
		case IndexZoomInOut:
			OnBtnZoomInout();
			break;
		case IndexMeasure:
			OnBtnMeasure();
			break;
		}
	}

	if (m_eButtonPressIndex != IndexNone)
	{
		m_eButtonPressIndex = IndexNone;
		InvalidateRect( m_rcMiniButton );
	}

	m_bIsButtonDown = FALSE;
	
	CWnd::OnLButtonUp(nFlags, point);
}

void CxImageViewCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	BOOL bIsMiniButtonPos = FALSE;
	for ( int i=0 ; i<sizeof(m_rectMiniBtnBody) / sizeof(Rect) ; i++ )
	{
		if ( m_rectMiniBtnBody[i].Contains(point.x, point.y) )
		{
			bIsMiniButtonPos = TRUE;
			break;
		}
	}

	if (!bIsMiniButtonPos)
	{
		BOOL bMaximized = IsMaximized();
		if ( m_rcTitle.PtInRect(point) && (m_dwMiniButtonType[bMaximized] & MBT_MAXIMIZE)  )
			OnBtnMaximize();
	}
	
	CWnd::OnLButtonDblClk(nFlags, point);
}

void CxImageViewCtrl::SyncDevContext( IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage )
{
	if ( m_pSyncManager )
	{
		m_pSyncManager->SyncDevContext( this, pIDC, ptImage, bUpdateImage );
	}
}

void CxImageViewCtrl::OnSyncDevContext( IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage )
{
	if ( m_pSyncManager && m_bSyncDevContext )
	{
//		TRACE( _T("%X - OnSyncDevContext: (%d, %d)\r\n"), this, ptImage.x, ptImage.y );

		float fZoomRatio;
		if ( m_pSyncManager->IsFixZoom() )
		{
			fZoomRatio = m_pImageView->GetZoomRatio();
		}
		else
		{
			fZoomRatio = pIDC->GetZoomRatio();
		}
		if ( m_pSyncManager->IsTrackMousePoint() )
		{
			ZoomTo( ptImage, fZoomRatio, TRUE );
		}
		else
		{
			if ( bUpdateImage )
			{
				int nX1, nY1, nX2, nY2;
				pIDC->GetCurrentViewingCoordinate( nX1, nY1, nX2, nY2 );
				//TRACE( _T("%X - OnSyncDevContext: (%d, %d)"), this, ptImage.x, ptImage.y );
				ZoomTo( CPoint((nX1+nX2)/2, (nY1+nY2)/2), pIDC->GetZoomRatio(), TRUE );
			}
		}
	}
}

void CxImageViewCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ( !IsWindowVisible() ) return;

	ButtonIconIndex eHoverIndex = IndexNone;
	for ( int i=0 ; i<sizeof(m_rectMiniBtnBody) / sizeof(Rect) ; i++ )
	{
		if ( m_rectMiniBtnBody[i].Contains(point.x, point.y) )
		{
			eHoverIndex = (ButtonIconIndex)i;
			if ( ::GetCapture() != m_hWnd )
			{
				::SetCapture(m_hWnd);
			}
			break;
		}
	}

	if ( eHoverIndex != m_eButtonHoverIndex )
	{
		if (eHoverIndex == IndexNone)
		{
			if ( !m_bIsButtonDown && ::GetCapture() == m_hWnd )
			{
				::ReleaseCapture();
			}
		}
		m_eButtonHoverIndex = eHoverIndex;
		InvalidateRect( m_rcMiniButton );
	}
	OnStatusText( GetTextFromIndex(m_eButtonHoverIndex) );
	
	if ( m_bEnableMoveWindow )
	{
		if ( (MK_LBUTTON & nFlags) != 0 )
		{
			ClientToScreen( &point );
			point -= m_ptLastMouse;

			SetWindowPos( NULL, point.x, point.y, 0, 0, SWP_NOSIZE );
		}
	}

	if (m_pImageView->IsUseAutoFocus())
		SetFocus();

	CWnd::OnMouseMove(nFlags, point);
}

void CxImageViewCtrl::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	if ( m_rcMaxWinPos.Width() <= 0 )
	{
		CWnd::OnGetMinMaxInfo(lpMMI);
		return;
	}
	
	CRect rcMaxWinPos = m_rcMaxWinPos;
	ClientToScreen( &rcMaxWinPos );
	lpMMI->ptMaxPosition.x = rcMaxWinPos.left;
	lpMMI->ptMaxPosition.y = rcMaxWinPos.top;
	lpMMI->ptMinTrackSize.x = rcMaxWinPos.Width(); 
    lpMMI->ptMinTrackSize.y = rcMaxWinPos.Height();
	lpMMI->ptMaxTrackSize.x = rcMaxWinPos.Width();
	lpMMI->ptMaxTrackSize.y = rcMaxWinPos.Height();
	lpMMI->ptMaxSize.x = rcMaxWinPos.Width();
	lpMMI->ptMaxSize.y = rcMaxWinPos.Height();

	CWnd::OnGetMinMaxInfo(lpMMI);
}

void APIENTRY CxImageViewCtrl::_OnDrawExt( IxDeviceContext* pIDC, CDC* pDC, UINT nIndexData, LPVOID lpUsrData )
{
	CxImageViewCtrl* pView = (CxImageViewCtrl*)lpUsrData;

	pView->LinkageIndexView( pIDC );

	if ( pView->m_fnOnDrawExt)
	{
		(*pView->m_fnOnDrawExt)( pIDC, pDC, nIndexData, pView->m_lpUsrDataOnDrawExt );
	}
}

void CxImageViewCtrl::SetRegisterCallBack( UINT nIndexData, REGISTER_CALLBACK* pRegisterCB )
{
	m_nIndexData = nIndexData;
	m_pRegisterCB = pRegisterCB;

	if ( m_pRegisterCB == NULL )
	{
		m_pImageView->SetOnDrawExt( _OnDrawExt, this );

		m_pImageView->SetOnEvent( m_fnOnEvent, m_lpUsrDataOnEvent );
		m_pImageView->SetOnFireMouseEvent( m_fnOnFireMouseEvent, m_lpUsrDataOnFirMouseEvent );
		m_pImageView->SetOnMeasure( m_fnOnMeasure, m_lpUsrDataOnMeasure );
		m_pImageView->SetOnConfirmTracker( m_fnOnConfirmTracker, m_lpUsrDataOnConfirmTracker );
	}
	else
	{
		m_fnOnDrawExt				= m_pRegisterCB->fnOnDrawExt;
		m_fnOnMeasure				= m_pRegisterCB->fnOnMeasure;
		m_fnOnConfirmTracker		= m_pRegisterCB->fnOnConfirmTracker;
		
		m_fnOnFireMouseEvent		= m_pRegisterCB->fnOnFireMouseEvent;
		m_fnOnEvent					= m_pRegisterCB->fnOnEvent;
		
		m_lpUsrDataOnDrawExt		= m_pRegisterCB->lpUsrData[0];
		m_lpUsrDataOnMeasure		= m_pRegisterCB->lpUsrData[1];
		m_lpUsrDataOnConfirmTracker	= m_pRegisterCB->lpUsrData[2];
		
		m_lpUsrDataOnFirMouseEvent	= m_pRegisterCB->lpUsrData[3];
		m_lpUsrDataOnEvent			= m_pRegisterCB->lpUsrData[4];
		
		m_pImageView->SetRegisterCallBack( m_nIndexData, m_pRegisterCB );
		m_pImageView->SetOnDrawExt( _OnDrawExt, this );
	}
}

void CxImageViewCtrl::LinkageIndexView( IxDeviceContext* pIDC )
{
	int nX1, nY1, nX2, nY2;
	pIDC->GetCurrentViewingCoordinate( nX1, nY1, nX2, nY2 );

	//m_wndIndexViewCtrl.SetActiveRect( CRect(nX1, nY1, nX2, nY2) );
}

BOOL CxImageViewCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_MOUSEWHEEL|| pMsg->message == WM_KEYDOWN )	
	{
		m_pImageView->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );

		CWnd* pWndOwner = GetOwner();
		if ( pWndOwner != NULL )
		{
			pWndOwner->SendMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
			return FALSE;
		}
		return TRUE;
	}

	return CWnd::PreTranslateMessage(pMsg);
}

void CxImageViewCtrl::OnStatusInfo( LONG lX, LONG lY, COLORREF dwColor, unsigned int nLevel, int nDepth, int nChannel )
{
	if ((m_ptPixel.x == lX) && (m_ptPixel.y == lY) && 
		(m_dwPixelColor == dwColor) &&
		(m_nPixelDepth == nDepth) &&
		(m_nChannel == nChannel) &&
		m_strStatus.IsEmpty() )
		return;
	m_ptPixel.x = lX; m_ptPixel.y = lY;
	m_dwPixelColor = dwColor;
	m_nPixelLevel = nLevel;
	m_nPixelDepth = nDepth;
	m_nChannel = nChannel;
	m_strStatus.Empty();
	RedrawStatus();
}

void CxImageViewCtrl::OnStatusText( LPCTSTR lpszText )
{
	if (m_strStatus.Compare(lpszText) == 0)
		return;
	m_strStatus = lpszText;
	RedrawStatus();
}

void CxImageViewCtrl::SetStatusColor( DWORD dwBodyColor, DWORD dwTextColor )
{
	if ((m_dwStatusBodyColor == dwBodyColor) &&
		(m_dwStatusTextColor == dwTextColor))
		return;
	m_dwStatusBodyColor = dwBodyColor;
	m_dwStatusTextColor = dwTextColor;
	RedrawStatus();
}

void CxImageViewCtrl::SetBodyColor( DWORD dwBodyColor )
{
	m_dwBodyColor = dwBodyColor;
	m_pImageView->SetBackgroundColor(m_dwBodyColor);
	if (m_BodyBrush.GetSafeHandle() != NULL)
		m_BodyBrush.DeleteObject();
	m_BodyBrush.CreateSolidBrush( m_dwBodyColor );
	Invalidate();
}

void CxImageViewCtrl::SetBorderColor( DWORD dwOutlineBorderColor, DWORD dwInnerTopBorderColor, DWORD dwInnerBottomBorderColor )
{
	m_dwBorderColors[0] = dwOutlineBorderColor;
	m_dwBorderColors[1] = dwInnerBottomBorderColor;
	m_dwBorderColors[2] = dwInnerTopBorderColor;
	Invalidate();
}

void CxImageViewCtrl::SetMiniButtonColorType( MiniButtonColorType type )
{
	if ( m_eMiniButtonColorType == type )
		return;
	m_eMiniButtonColorType = type;
	switch (m_eMiniButtonColorType)
	{
	case MiniButtonColorTypeBlack:
		m_pMiniBtnIcon = m_pMiniBtnIconBlack;
		break;
	case MiniButtonColorTypeWhite:
		m_pMiniBtnIcon = m_pMiniBtnIconWhite;
		break;
	case MiniButtonColorTypeColor:
		m_pMiniBtnIcon = m_pMiniBtnIconColor;
		break;
	}
	Invalidate();
}

void CxImageViewCtrl::GetTitleRect( LPRECT lpRect )
{
	::CopyRect( lpRect, m_rcTitle );
}

void CxImageViewCtrl::SetTitleColor( DWORD dwBodyColor, DWORD dwInactiveTextColor, DWORD dwActiveTextColor )
{
	m_dwTitleBodyColor = dwBodyColor;
	m_dwActiveTitleColor = dwActiveTextColor;
	m_dwInactiveTitleColor = dwInactiveTextColor;
	if (m_pImageView && *m_pImageView)
		m_pImageView->SetPopupMenuColor( m_dwTitleBodyColor, m_dwActiveTitleColor );
	RedrawTitle();
}

void CxImageViewCtrl::SetButtonColor( DWORD dwHoverColor, DWORD dwPressColor,
									  DWORD dwBorderHoverColor, DWORD dwBorderPressColor )
{
	m_dwButtonHoverColor = dwHoverColor;
	m_dwButtonPressColor = dwPressColor;
	m_dwButtonBorderHoverColor = dwBorderHoverColor;
	m_dwButtonBorderPressColor = dwBorderPressColor;
}

CxImageViewCtrl::ButtonIconIndex CxImageViewCtrl::GetIndexFromBtnMask( DWORD dwType )
{
	switch (dwType)
	{
	case MBT_ZOOM_IN:
		return IndexZoomIn;
	case MBT_ZOOM_OUT:
		return IndexZoomOut;
	case MBT_ZOOM_FIT:
		return IndexZoomFit;
	case MBT_ZOOM_NOT:
		return IndexZoomNot;
	case MBT_ZOOM_INOUT:
		return IndexZoomInOut;
	case MBT_MEASURE:
		return IndexMeasure;
	case MBT_LOAD:
		return IndexLoad;
	case MBT_SAVE:
		return IndexSave;
	case MBT_MAXIMIZE:
		if (IsMaximized())
			return IndexShowNormal;
		else
			return IndexShowMaximize;
	}
	return IndexNone;
}

CString CxImageViewCtrl::GetTextFromIndex(ButtonIconIndex index)
{
	CString strText;
	switch (index)
	{
	case IndexZoomIn:
		strText.LoadString(GetResourceHandle(), IDS_ZOOM_IN);
		break;
	case IndexZoomOut:
		strText.LoadString(GetResourceHandle(), IDS_ZOOM_OUT);
		break;
	case IndexZoomInOut:
		strText.LoadString(GetResourceHandle(), IDS_ZOOM_INOUT);
		break;
	case IndexZoomFit:
		strText.LoadString(GetResourceHandle(), IDS_ZOOM_FIT);
		break;
	case IndexZoomNot:
		strText.LoadString(GetResourceHandle(), IDS_ZOOM_NOT);
		break;
	case IndexLoad:
		strText.LoadString(GetResourceHandle(), IDS_LOAD_IMAGE);
		break;
	case IndexSave:
		strText.LoadString(GetResourceHandle(), IDS_SAVE_IMAGE);
		break;
	case IndexMeasure:
		strText.LoadString(GetResourceHandle(), IDS_MEASURE);
		break;
	case IndexShowIndexView:
		strText.LoadString(GetResourceHandle(), IDS_INDEX_VIEW);
		break;
	case IndexShowNormal:
		strText.LoadString(GetResourceHandle(), IDS_RESTORE_VIEW);
		break;
	case IndexShowMaximize:
		strText.LoadString(GetResourceHandle(), IDS_MAXIMIZE_VIEW);
		break;
	case IndexMore:
		strText.LoadString(GetResourceHandle(), IDS_MORE);
		break;
	}

	return strText;
}

void CxImageViewCtrl::DrawTitle( Gdiplus::Graphics& g )
{
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

	DWORD dwBtnType = m_dwMiniButtonType[IsMaximized() ? 1 : 0];
	DWORD dwType = 0x01;
	int nBtnCnt = 0;
	BOOL bFirst = FALSE;
	DWORD dwLastBtnType = 0x00;
	while ( dwType != MBT_LIMIT )
	{
		if ( (dwType & dwBtnType) != 0 )
		{
			++nBtnCnt;
			if (!bFirst)
			{
				bFirst = TRUE;
				m_eButtonFirst = GetIndexFromBtnMask(dwType);
			}
			dwLastBtnType = dwType;
		}
		dwType <<= 1;
	}

	m_eButtonLast = GetIndexFromBtnMask(dwLastBtnType);

	const int nOffsetX = 5;
	Color colorTitleBody;
	colorTitleBody.SetFromCOLORREF(m_dwTitleBodyColor);
	//GraphicsPath* pBodyPath = ::GdipCreateRoundRect( Rect(m_rcTitle.left-1, m_rcTitle.top-1, m_rcTitle.Width()+1, m_rcTitle.Height()+1), 6, 6, 0, 0 );
	//g.FillPath( &SolidBrush(colorTitleBody), pBodyPath );
	g.FillRectangle( &SolidBrush(colorTitleBody), m_rcTitle.left-1, m_rcTitle.top-1, m_rcTitle.Width()+1, m_rcTitle.Height()+1 );
	//delete pBodyPath;

	int nIconX=0, nIconY=0, nIconW=0, nIconH=0;
	if (m_bShowTitleIcon)
	{
		nIconW = m_nMiniBtnIconSize;
		nIconH = m_nMiniBtnIconSize;
		nIconX = m_rcTitle.left+nOffsetX;
		nIconY = (int)(( m_rcTitle.Height() - nIconH ) / 2 + m_rcTitle.top);
		g.DrawImage( m_pMiniBtnIcon, Rect(nIconX, nIconY, nIconW, nIconH), IndexCamera*nIconW, 0, nIconW, nIconH , UnitPixel);
	}

	const int nBtnInterval = 2;
	const int nBtnSize = m_nMiniBtnIconSize + 6;
	int nButtonsWidth = (nBtnCnt*(nBtnSize) + (nBtnCnt-1)*nBtnInterval);
	int nButtonsHeight = nBtnSize;

	if (nButtonsWidth < 0) nButtonsWidth = 0;

	Color colorTitle;
	if (m_bInActive)
	{
		colorTitle.SetValue(
			Color::MakeARGB(
				240, 
				GetRValue(m_dwInactiveTitleColor), 
				GetGValue(m_dwInactiveTitleColor), 
				GetBValue(m_dwInactiveTitleColor)
			)
		);
	}
	else
	{
		colorTitle.SetValue(
			Color::MakeARGB(
				250, 
				GetRValue(m_dwActiveTitleColor), 
				GetGValue(m_dwActiveTitleColor), 
				GetBValue(m_dwActiveTitleColor)
			)
		);
	}
	//colorTitle.SetFromCOLORREF( m_bInActive ? m_dwInactiveTitleColor : m_dwActiveTitleColor );
	SolidBrush brushTitle( colorTitle );
	StringFormat stringFormat;
	stringFormat.SetAlignment( StringAlignmentNear );
	stringFormat.SetLineAlignment( StringAlignmentCenter );
	stringFormat.SetFormatFlags(StringFormatFlagsLineLimit|StringFormatFlagsNoWrap);
	stringFormat.SetTrimming( StringTrimmingEllipsisCharacter );
	BSTR bstrTitle = m_strTitle.AllocSysString();
	RectF rectTitle(
		(REAL)m_rcTitle.left + nOffsetX + nIconW, 
		(REAL)m_rcTitle.top, 
		(REAL)m_rcTitle.Width() - nIconW - nOffsetX*2 - nButtonsWidth - (m_rcTitle.Height() - nButtonsHeight), 
		(REAL)m_rcTitle.Height());

	Font fontTitle(m_pFontFamily, 12, FontStyleBold, UnitPixel);
	g.DrawString( bstrTitle, -1, &fontTitle, rectTitle, &stringFormat, &brushTitle );
	SysFreeString( bstrTitle );

	if (nBtnCnt == 0 || !m_bShowMiniButtons)
		return;

	RectF rectButtons;
	rectButtons.X = m_rcTitle.right - nButtonsWidth - (m_rcTitle.Height() - nButtonsHeight)/2.f;
	rectButtons.Width = (float)nButtonsWidth;
	rectButtons.Y = m_rcTitle.top + (m_rcTitle.Height() - nButtonsHeight)/2.f;
	rectButtons.Height = (float)nButtonsHeight-1.f;

	m_rcMiniButton.left = (int)(rectButtons.GetLeft()+0.5f);
	m_rcMiniButton.top = (int)(rectButtons.GetTop()+0.5f);
	m_rcMiniButton.right = (int)(rectButtons.GetRight()+0.5f);
	m_rcMiniButton.bottom = (int)(rectButtons.GetBottom()+0.5f);

	m_rcMiniButton.InflateRect( 1, 1, 1, 1 );

	RectF rectBtnBody = rectButtons;
	//rectBtnBody.Inflate( 1, 0 );
	rectBtnBody.X -= 1.f;
	rectBtnBody.Width += 1.f;

#if 0
	GraphicsPath* pRoundRect = GdipCreateRoundRect( rectBtnBody, 2, 2, 2, 2 );
	g.FillPath( &SolidBrush(Color(20,255,255,255)), pRoundRect );
	g.DrawPath( &Pen(Color(80,255,255,255)), pRoundRect );
	delete pRoundRect;


	float fBX = rectButtons.X + nBtnSize + (float)nBtnInterval/2;
	float fBT = (float)rectButtons.Y + 0.5f;
	float fBB = (float)rectButtons.GetBottom() - 0.5f;

	for ( int i=0 ; i<nBtnCnt-1 ; i++ )
	{
		g.DrawLine( &Pen(Color(80,255,255,255)), fBX, fBT, fBX, fBB );
		fBX += nBtnSize+nBtnInterval;
	}
#endif
	RectF rectBtn;
	rectBtn.Y = (float)rectButtons.Y + 0.5f;
	rectBtn.Height = rectButtons.Height - 1.0f;
	rectBtn.X = (float)rectButtons.X;
	rectBtn.Width = (float)nBtnSize;

	if ( (dwBtnType & MBT_ZOOM_IN) == MBT_ZOOM_IN )
	{
		DrawMiniButton( g, rectBtn, IndexZoomIn );

		m_rectMiniBtnBody[IndexZoomIn].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexZoomIn].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexZoomIn].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexZoomIn].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}
	if ( (dwBtnType & MBT_ZOOM_OUT) == MBT_ZOOM_OUT )
	{
		DrawMiniButton( g, rectBtn, IndexZoomOut );

		m_rectMiniBtnBody[IndexZoomOut].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexZoomOut].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexZoomOut].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexZoomOut].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}
	if ( (dwBtnType & MBT_ZOOM_INOUT) == MBT_ZOOM_INOUT )
	{
		DrawMiniButton( g, rectBtn, IndexZoomInOut );

		m_rectMiniBtnBody[IndexZoomInOut].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexZoomInOut].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexZoomInOut].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexZoomInOut].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}

	if ( (dwBtnType & MBT_ZOOM_FIT) == MBT_ZOOM_FIT )
	{
		DrawMiniButton( g, rectBtn, IndexZoomFit );

		m_rectMiniBtnBody[IndexZoomFit].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexZoomFit].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexZoomFit].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexZoomFit].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}
	if ( (dwBtnType & MBT_ZOOM_NOT) == MBT_ZOOM_NOT )
	{
		DrawMiniButton( g, rectBtn, IndexZoomNot );

		m_rectMiniBtnBody[IndexZoomNot].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexZoomNot].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexZoomNot].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexZoomNot].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}

	if ( (dwBtnType & MBT_MEASURE) == MBT_MEASURE )
	{
		DrawMiniButton( g, rectBtn, IndexMeasure );

		m_rectMiniBtnBody[IndexMeasure].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexMeasure].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexMeasure].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexMeasure].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}

	if ( (dwBtnType & MBT_LOAD) == MBT_LOAD )
	{
		DrawMiniButton( g, rectBtn, IndexLoad );

		m_rectMiniBtnBody[IndexLoad].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexLoad].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexLoad].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexLoad].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}
	if ( (dwBtnType & MBT_SAVE) == MBT_SAVE )
	{
		DrawMiniButton( g, rectBtn, IndexSave );

		m_rectMiniBtnBody[IndexSave].X = (int)(rectBtn.X+0.5f);
		m_rectMiniBtnBody[IndexSave].Y = (int)(rectBtn.Y+0.5f);
		m_rectMiniBtnBody[IndexSave].Width = (int)rectBtn.Width;
		m_rectMiniBtnBody[IndexSave].Height = (int)rectBtn.Height;

		rectBtn.X += nBtnInterval + nBtnSize;
	}

	if ( (dwBtnType & MBT_MAXIMIZE) == MBT_MAXIMIZE )
	{
		DrawMiniButton( g, rectBtn, IsMaximized() ? IndexShowNormal : IndexShowMaximize );

		if ( IsMaximized() )
		{
			m_rectMiniBtnBody[IndexShowNormal].X = (int)(rectBtn.X+0.5f);
			m_rectMiniBtnBody[IndexShowNormal].Y = (int)(rectBtn.Y+0.5f);
			m_rectMiniBtnBody[IndexShowNormal].Width = (int)rectBtn.Width;
			m_rectMiniBtnBody[IndexShowNormal].Height = (int)rectBtn.Height;
			m_rectMiniBtnBody[IndexShowMaximize].X = -1;
		}
		else
		{
			m_rectMiniBtnBody[IndexShowMaximize].X = (int)(rectBtn.X+0.5f);
			m_rectMiniBtnBody[IndexShowMaximize].Y = (int)(rectBtn.Y+0.5f);
			m_rectMiniBtnBody[IndexShowMaximize].Width = (int)rectBtn.Width;
			m_rectMiniBtnBody[IndexShowMaximize].Height = (int)rectBtn.Height;
			m_rectMiniBtnBody[IndexShowNormal].X = -1;
		}

		rectBtn.X += nBtnInterval + nBtnSize;
	}
}

void CxImageViewCtrl::DrawMiniButton( Gdiplus::Graphics& g, const Gdiplus::RectF& rectBtn, ButtonIconIndex eIndex )
{
	int nIconX, nIconY, nIconW, nIconH;
	nIconW = m_nMiniBtnIconSize;
	nIconH = m_nMiniBtnIconSize;
	nIconX = (int)( (rectBtn.Width - nIconW) / 2 + rectBtn.GetLeft() );
	nIconY = (int)( (rectBtn.Height - nIconH) / 2 + rectBtn.GetTop() ) + 1;

	BOOL bIsChecked = FALSE;
	BOOL bIsPressed = FALSE;
	BOOL bIsHovered = FALSE;
	if ( eIndex == m_eZoomCheckedIndex )
	{
		bIsChecked = TRUE;
	}
	if ( eIndex == m_eButtonHoverIndex )
	{
		bIsHovered = TRUE;
	}
	if ( eIndex == m_eButtonPressIndex )
	{
		bIsHovered = FALSE;
		bIsPressed = TRUE;
	}

	Color colorButton = Color(0, GetRValue(m_dwButtonHoverColor), GetGValue(m_dwButtonHoverColor), GetBValue(m_dwButtonHoverColor));
	if (bIsChecked || bIsPressed)
		colorButton = Color(180, GetRValue(m_dwButtonPressColor), GetGValue(m_dwButtonPressColor), GetBValue(m_dwButtonPressColor));

	RectF rectBody = rectBtn;
	if ( bIsHovered )
	{
		colorButton = Color(colorButton.GetA()+50, colorButton.GetR(), colorButton.GetG(), colorButton.GetB());
	}
	else if ( bIsPressed )
	{
		BYTE r = colorButton.GetR() > 50 ? colorButton.GetR() - 50 : 0;
		BYTE g = colorButton.GetG() > 50 ? colorButton.GetG() - 50 : 0;
		BYTE b = colorButton.GetB() > 50 ? colorButton.GetB() - 50 : 0;
		colorButton = Color(colorButton.GetA(), r, g, b);
	}

	SolidBrush brushBody(colorButton);
	{
		rectBody.X -= 0.5f;
		rectBody.Width += 1.f;
		g.FillRectangle( &brushBody, rectBody );
	}
	if (bIsPressed)
	{
		nIconY++;
	}

	float fAlpha = (bIsChecked || bIsPressed || bIsHovered) ? 0.9f : 0.7f;
	ImageAttributes ImgAttr;
	ColorMatrix ClrMatrix = 
	{ 
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, fAlpha, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	ImgAttr.SetColorMatrix(&ClrMatrix, ColorMatrixFlagsDefault, 
		ColorAdjustTypeBitmap);

	OSVERSIONINFO ovi;
	ovi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL bRet = GetVersionEx(&ovi);
	BOOL bIsLowerXP = FALSE;
	if (bRet)
	{
		if (ovi.dwMajorVersion <= 5)	// XP: 5.1, 2000: 5.0, Vista: 6.0
		{
			bIsLowerXP = TRUE;
		}
	}

	if (bIsLowerXP)
		g.DrawImage( m_pMiniBtnIcon, Rect(nIconX, nIconY, nIconW, nIconH), eIndex*nIconW, 0, nIconW, nIconH, UnitPixel );
	else
		g.DrawImage( m_pMiniBtnIcon, Rect(nIconX, nIconY, nIconW, nIconH), eIndex*nIconW, 0, nIconW, nIconH, UnitPixel, &ImgAttr );
}

void CxImageViewCtrl::DrawStatus( Gdiplus::Graphics& g )
{
	const int nOffsetX = 5;

	Color colorStatusBody;
	colorStatusBody.SetFromCOLORREF(m_dwStatusBodyColor);

	//GraphicsPath* pBodyPath = ::GdipCreateRoundRect( Rect(m_rcStatus.left, m_rcStatus.top, m_rcStatus.Width(), m_rcStatus.Height()), 0, 0, 9, 8 );
	//g.FillPath( &SolidBrush(colorStatusBody), pBodyPath );
	//delete pBodyPath;

	g.FillRectangle( &SolidBrush(colorStatusBody), m_rcStatus.left, m_rcStatus.top, m_rcStatus.Width(), m_rcStatus.Height() );

	Color colorStatus;
	colorStatus.SetFromCOLORREF( m_dwStatusTextColor );
	SolidBrush brushStatus( colorStatus );
	StringFormat stringFormat;
	stringFormat.SetAlignment( StringAlignmentNear );
	stringFormat.SetLineAlignment( StringAlignmentCenter );
	stringFormat.SetFormatFlags(StringFormatFlagsLineLimit|StringFormatFlagsNoWrap);
	stringFormat.SetTrimming( StringTrimmingEllipsisCharacter );

	Font fontStatus(m_pFontFamily, 11, FontStyleBold, UnitPixel);

	RectF rectPixelInfo((REAL)m_rcStatus.left+nOffsetX, (REAL)m_rcStatus.top, (REAL)m_rcStatus.Width()/2-nOffsetX, (REAL)m_rcStatus.Height());

	if ( m_strStatus.IsEmpty() )
	{
		int nGVWH = m_rcStatus.Height() / 2;
		RectF rectGV(0.f, 0.f, (float)nGVWH, (float)nGVWH);

		rectGV.X = rectPixelInfo.X;
		rectGV.Y = (rectPixelInfo.Height-nGVWH) / 2.f + rectPixelInfo.Y;

		Color colorGV;
		colorGV.SetFromCOLORREF( m_dwPixelColor );
		g.FillRectangle( &SolidBrush(colorGV), rectGV );

		rectPixelInfo.X += rectGV.Width + 2;
		rectPixelInfo.Width -= rectGV.Width + 2;

		CString strLevel;
		if (m_nChannel == 1)
		{
			strLevel.Format( _T("[%d, %d] = %d"), m_ptPixel.x, m_ptPixel.y, m_nPixelLevel );
		}
		else
		{
			strLevel.Format( _T("[%d, %d] = (%d, %d, %d)"), m_ptPixel.x, m_ptPixel.y, GetRValue(m_dwPixelColor), GetGValue(m_dwPixelColor), GetBValue(m_dwPixelColor) );
		}
		BSTR bstrLevel = strLevel.AllocSysString();
		g.DrawString( bstrLevel, -1, &fontStatus, rectPixelInfo, &stringFormat, &brushStatus );
		SysFreeString(bstrLevel);
	}
	else
	{
		BSTR bstrStatus = m_strStatus.AllocSysString();
		g.DrawString( bstrStatus, -1, &fontStatus, rectPixelInfo, &stringFormat, &brushStatus );
		SysFreeString(bstrStatus);
	}

	RectF rectZoom((REAL)m_rcStatus.left+(REAL)m_rcStatus.Width()/2, (REAL)m_rcStatus.top, (REAL)m_rcStatus.Width()/2-nOffsetX, (REAL)m_rcStatus.Height());

	CString strZoom;
	strZoom.Format( _T("1 : %.3f"), m_pImageView->GetZoomRatio() );

	BSTR bstrZoom = strZoom.AllocSysString();
	stringFormat.SetAlignment( StringAlignmentFar );
	g.DrawString( bstrZoom, -1, &fontStatus, rectZoom, &stringFormat, &brushStatus );

	SysFreeString(bstrZoom);
}

void CxImageViewCtrl::DrawBorder( Gdiplus::Graphics& g )
{
	CRect rc;
    GetClientRect(rc);

	if (m_dwBorderColors[2] != (DWORD)-1)
	{
		Color colorInnerBorder;
		colorInnerBorder.SetFromCOLORREF(m_dwBorderColors[2]);
		Pen penInnerBorder(colorInnerBorder);
		g.DrawLine( &penInnerBorder, m_rcStatus.left, m_rcStatus.top+1, m_rcStatus.right, m_rcStatus.top+1 );
	}

	if (m_dwBorderColors[1] != (DWORD)-1)
	{
		Color colorInnerBorder;
		colorInnerBorder.SetFromCOLORREF(m_dwBorderColors[1]);
		Pen penInnerBorder(colorInnerBorder);
		g.DrawLine( &penInnerBorder, m_rcTitle.left, m_rcTitle.bottom-1, m_rcTitle.right, m_rcTitle.bottom-1 );
	}

	if (m_dwBorderColors[0] != (DWORD)-1)
	{
		Color colorOutlineBorder;
		colorOutlineBorder.SetFromCOLORREF(m_dwBorderColors[0]);
		Pen penOutlineBorder(colorOutlineBorder);
		g.DrawRectangle( &penOutlineBorder, rc.left, rc.top, rc.Width()-1, rc.Height()-1 );
	}
}

void CxImageViewCtrl::OnDraw( CDC& dc )
{
	Graphics g(dc.GetSafeHdc());

	if (m_bShowTitle)
	{
		DrawTitle( g );
	}

	if (m_bShowStatus)
	{
		DrawStatus( g );
	}

	DrawBorder( g );
}

void CxImageViewCtrl::OnPaint() 
{
	CPaintDC dc(this);

	CRect rtClient;
	GetClientRect(&rtClient);

	if ( !m_pBitmap )
	{
		m_pBitmap = new CBitmap;
		ASSERT( m_pBitmap );
		VERIFY( m_pBitmap->CreateCompatibleBitmap(&dc, rtClient.Width(), rtClient.Height()) );
	}

	if ( m_MemDC.GetSafeHdc() == NULL )
	{
		m_MemDC.CreateCompatibleDC(&dc);
	}

	if ( !m_bCreateInnerControl )
	{
		if ( m_pViewManager )
			m_pViewManager->AddView( this );
		if ( m_pSyncManager )
			m_pSyncManager->AddView( this );
		CreateInnerCtrls();
		ArrangeInnerView( rtClient.Width(), rtClient.Height() );
	}

	CBitmap * pOldBitmap = m_MemDC.SelectObject(m_pBitmap);

	OnDraw(m_MemDC);

	m_MemDC.SetMapMode(MM_TEXT);
	m_MemDC.SetViewportOrg(0, 0);

	dc.BitBlt(rtClient.left, rtClient.top,
		rtClient.Width(), rtClient.Height(),
		&m_MemDC, 0, 0, SRCCOPY);
	
	m_MemDC.SelectObject(pOldBitmap);
}

void CxImageViewCtrl::OnKillFocus(CWnd* pNewWnd)
{
	m_bInActive = TRUE;
	RedrawTitle();
}

void CxImageViewCtrl::OnSetFocus(CWnd* pOldWnd)
{
	m_bInActive = FALSE;
	RedrawTitle();

	GetOwner()->ActivateTopParent();
}

void CxImageViewCtrl::RedrawTitle()
{
	if (!GetSafeHwnd()) return;
	if ( m_bShowTitle )
		InvalidateRect( m_rcTitle );
}

void CxImageViewCtrl::RedrawStatus()
{
	if (!GetSafeHwnd()) return;
	if ( m_bShowStatus )
		InvalidateRect( m_rcStatus );
}

void CxImageViewCtrl::SetPalette( const BYTE* pPal )
{
	if ( m_pImageView )
	{
		m_pImageView->SetPalette( pPal );
	}
}

void CxImageViewCtrl::OnTimer(UINT_PTR nIDEvent) 
{

	CWnd::OnTimer(nIDEvent);
}
