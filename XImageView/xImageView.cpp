/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */
#include "stdafx.h"
#include <XImageView/xImageView.h>
#include "resource.h"
#include <XImage/xImageObject.h>

#include <XUtil/xUtils.h>
#include <XUtil/xEvent.h>
#include <XUtil/xCriticalSection.h>
#include <XUtil/xThread.h>

#include <math.h>
#include <float.h>
#define WM_MOUSEENTER		(WM_USER+100)

#include <XImageView/xImageViewCtrl.h>
#include <XUtil/xStopWatch.h>

#include "xDirectDIB.h"
#include "xDrawDIB.h"
#include <XGraphic/xArrowDrawer.h>
#include "LineTracker.h"
#include "SidebarMenu.h"
#include "ColorRectTracker.h"

#define ID_TRACKER_CLEAR							10
#define ID_TRACKER_CONFIRM_REGION					11
#define ID_WHEEL_SET_TO_VSCROLL						12
#define ID_WHEEL_SET_TO_HSCROLL						13
#define ID_WHEEL_SET_TO_ZOOM						14
#define ID_WHEELBTN_SET_TO_SMART_MOVE				15
#define ID_LBUTTON_LIMIT_LOCK						16
#define ID_TRACKER_START							ID_TRACKER_CLEAR
#define ID_TRACKER_END								ID_LBUTTON_LIMIT_LOCK

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CInnerUI
{
public:
	CLineTracker				m_LineTracker;
	CLineTracker::CLineList		m_LineList;
	CColorRectTracker			m_RectTracker;
	CSidebarMenu				m_ROIPopupMenu;
	CSidebarMenu				m_WheelPopupMenu;
	BOOL						m_bFixedTracker;
	CxArrowDrawer				m_MeasureArrowDrawer;
};

#include <locale.h>

IMPLEMENT_DYNCREATE(CxImageView, CView)

CxImageView::CxImageView() : 
	m_pImageObject(NULL), m_pRenderer(NULL), m_fZoomRatio(1.f),
	m_pDC(NULL), m_eScreenMode(ImageViewMode::ScreenModeNone),
	m_fZoomMin(1.f), m_fZoomMax(1.f), m_fZoomFit(1.f),
	m_nBodyOffsetX(0), m_nBodyOffsetY(0), m_bLockUpdate(FALSE), m_GraphicObject(this),
	m_fnOnDrawExt(NULL), m_lpUsrDataOnDrawExt(NULL),
	m_fnOnMeasure(NULL), m_lpUsrDataOnMeasure(NULL),
	m_fnOnConfirmTracker(NULL), m_lpUsrDataOnConfirmTracker(NULL),
	m_fnOnFireMouseEvent(NULL), m_lpUsrDataOnFireMouseEvent(NULL),
	m_fnOnEvent(NULL), m_lpUsrDataOnEvent(NULL), m_nIndexData(0),
	m_eMouseWheelMode(ImageViewMode::MouseWheelModeZoom), m_bEnableSmartMove(FALSE), m_bEnableLockMouse(FALSE),
	m_bShowScaleBar(TRUE),
	m_bShowDigitize(TRUE), m_bMouseOverCheck(FALSE)
{
#ifdef USE_MEMDC_IMAGE_VIEW
	m_pBitmap = NULL;
	m_pOldBitmap = NULL;
#endif

	m_ptScrollPos = CPoint(0,0);
	m_sizeScrollRange = CSize(0,0);
	m_nTimerAutoHideScrollBar = 0;
	m_ptPrevDown = CPoint(0,0);
	m_ptOldScrollPos = CPoint(0,0);

	m_rcScrollThumbX.SetRectEmpty();
	m_rcScrollThumbY.SetRectEmpty();
    m_bControlScrollBar = FALSE;

	m_pInnerUI = new CInnerUI();
	m_pDirectDIB = new CxDirectDIB();
	m_pDrawDIB = new CxDrawDIB();
	m_pInnerUI->m_bFixedTracker = FALSE;
	m_dDrawElapsedTime = 0.0;
	m_bShowDrawElapsedTime = FALSE;

	m_cClearColor = 128;
	m_dwBackgroundColor = RGB(128,128,128);

	m_bUseAutoFocus = TRUE;
	m_bEnableMouseControl = TRUE;

	m_nWidth = m_nHeight = 0;
	m_nBitCnt = 0;
	m_nWidthBytes = 0;

	m_ptViewLastMouse.x = m_ptViewLastMouse.y = -1;

	HMODULE hResDll;
	hResDll = GetResourceHandle();

	if ( hResDll )
	{
		m_hCursorZoomNot = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_NOT), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorZoomIn = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_IN), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorZoomOut = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_OUT), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorPan = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_PAN), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorZoomInOut = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_ZOOM_INOUT), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorNormal = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_NORMAL), IMAGE_CURSOR, 0, 0, 0 );
		m_hCursorTracker = (HCURSOR)::LoadImage( hResDll, MAKEINTRESOURCE(IDC_CURSOR_TRACKER), IMAGE_CURSOR, 0, 0, 0 );
	}

	m_fRealPixelSizeW = 7.f;			// 7um
	m_fRealPixelSizeH = 7.f;			// 7um

	m_pInnerUI->m_MeasureArrowDrawer.Create( 1, RGB(0xff,0,0), RGB(0,0,0), RGB(255,150,150) );

	m_pInnerUI->m_RectTracker.m_nStyle = CRectTracker::solidLine|CRectTracker::hatchedBorder|CRectTracker::resizeOutside|CRectTracker::resizeInside;
	m_pInnerUI->m_RectTracker.m_sizeMin = CSize( 10, 10 );


#ifdef _DEBUG
_tsetlocale(LC_ALL, _T("korean"));
#endif

	CString strText;
	m_pInnerUI->m_ROIPopupMenu.CreatePopupMenu();
	strText.LoadString(GetResourceHandle(), IDS_ROI_POPUP_TITLE);
	m_pInnerUI->m_ROIPopupMenu.SetSideBarText( strText );
	m_pInnerUI->m_ROIPopupMenu.SetMenuTextColor( RGB(0,0,0) );
	m_pInnerUI->m_ROIPopupMenu.SetHightlightColor( RGB(0, 88, 176) );
	
	static CString strTrackerClear;
	strTrackerClear.LoadString(GetResourceHandle(), IDS_TRACKER_CLEAR);
	m_pInnerUI->m_ROIPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_TRACKER_CLEAR, strTrackerClear );
	m_pInnerUI->m_ROIPopupMenu.AppendMenu( MFT_SEPARATOR|MFT_OWNERDRAW, 0, _T("") );
	static CString strTrackerConfirmRegion;
	strTrackerConfirmRegion.LoadString(GetResourceHandle(), IDS_TRACKER_CONFIRM_REGION);
	m_pInnerUI->m_ROIPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_TRACKER_CONFIRM_REGION, strTrackerConfirmRegion );
		
	m_pInnerUI->m_WheelPopupMenu.CreatePopupMenu();
	strText.LoadString(GetResourceHandle(), IDS_SETTING_POPUP_TITLE);
	m_pInnerUI->m_WheelPopupMenu.SetSideBarText( strText );
	m_pInnerUI->m_WheelPopupMenu.SetMenuTextColor( RGB(0,0,0) );
	m_pInnerUI->m_WheelPopupMenu.SetHightlightColor( RGB(0, 88, 176) );
	
	static CString strWheelSetToZoom;
	strWheelSetToZoom.LoadString(GetResourceHandle(), IDS_WHEEL_SET_TO_ZOOM);
	m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_WHEEL_SET_TO_ZOOM, strWheelSetToZoom );
	m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_SEPARATOR|MFT_OWNERDRAW, 0, _T("") );
	static CString strWheelSetToVScroll;
	strWheelSetToVScroll.LoadString(GetResourceHandle(), IDS_WHEEL_SET_TO_VSCROLL);
	m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_WHEEL_SET_TO_VSCROLL, strWheelSetToVScroll );
	static CString strWheelSetToHScroll;
	strWheelSetToHScroll.LoadString(GetResourceHandle(), IDS_WHEEL_SET_TO_HSCROLL);
	m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_WHEEL_SET_TO_HSCROLL, strWheelSetToHScroll );
	//static CString strWheelSetToSmartMove;
	//strWheelSetToSmartMove.LoadString(GetResourceHandle(), IDS_WHEEL_SET_TO_SMART_MOVE);
	//m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_WHEELBTN_SET_TO_SMART_MOVE, strWheelSetToSmartMove );
	//m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_SEPARATOR|MFT_OWNERDRAW, 0, _T("") );
	//static CString strLButtonLimitLock;
	//strLButtonLimitLock.LoadString(GetResourceHandle(), IDS_LBUTTON_LIMIT_LOCK);
	//m_pInnerUI->m_WheelPopupMenu.AppendMenu( MFT_STRING|MFT_OWNERDRAW, ID_LBUTTON_LIMIT_LOCK, strLButtonLimitLock );
}

CxImageView::~CxImageView()
{
#ifdef USE_MEMDC_IMAGE_VIEW
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap=NULL;
	}
#endif

	delete m_pInnerUI;
	delete m_pDirectDIB;
	delete m_pDrawDIB;

	::DestroyCursor( m_hCursorZoomIn );
	::DestroyCursor( m_hCursorZoomInOut );
	::DestroyCursor( m_hCursorZoomOut );
	::DestroyCursor( m_hCursorZoomNot );
	::DestroyCursor( m_hCursorPan );
	::DestroyCursor( m_hCursorNormal );
	::DestroyCursor( m_hCursorTracker );
}

BEGIN_MESSAGE_MAP(CxImageView, CView)
	ON_WM_MOUSEACTIVATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_KEYDOWN()
	ON_WM_NCDESTROY()
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_COMMAND_RANGE( ID_TRACKER_START, ID_TRACKER_END, OnContextMenuHandler )
	ON_WM_PAINT()
END_MESSAGE_MAP()

float CxImageView::CalcZoomValid( float fZoom )
{
	if ( fZoom == 1.f ) return fZoom;
	if ( m_fZoomMax < fZoom ) return m_fZoomMax;
	if ( m_fZoomMin > fZoom ) return m_fZoomMin;
	return fZoom;
}

void CxImageView::SetOnDrawExt( FnOnDrawExt _fnOnDrawExt, LPVOID _lpUsrData ) 
{ m_fnOnDrawExt = _fnOnDrawExt; m_lpUsrDataOnDrawExt = _lpUsrData; }
void CxImageView::SetOnMeasure( FnOnMeasure _fnOnMeasure, LPVOID _lpUsrData ) 
{ m_fnOnMeasure = _fnOnMeasure; m_lpUsrDataOnMeasure = _lpUsrData; }
void CxImageView::SetOnConfirmTracker( FnOnConfirmTracker _fnOnConfirmTracker, LPVOID _lpUsrData ) 
{ m_fnOnConfirmTracker = _fnOnConfirmTracker; m_lpUsrDataOnConfirmTracker = _lpUsrData; }
void CxImageView::SetOnFireMouseEvent( FnOnFireMouseEvent _fnOnFireMouseEvent, LPVOID _lpUsrData )
{ m_fnOnFireMouseEvent = _fnOnFireMouseEvent; m_lpUsrDataOnFireMouseEvent = _lpUsrData; }
void CxImageView::SetOnEvent( FnOnEvent _fnOnEvent, LPVOID _lpUsrData )
{ m_fnOnEvent = _fnOnEvent; m_lpUsrDataOnEvent = _lpUsrData; }

float CxImageView::GetRealSizePerPixel() const { return m_fRealPixelSizeW; }
void CxImageView::SetRealSizePerPixel( float fRealPixelSize ) { m_fRealPixelSizeW = fRealPixelSize; m_fRealPixelSizeH = fRealPixelSize; }
float CxImageView::GetRealSizePerPixelW() const { return m_fRealPixelSizeW; }
void CxImageView::SetRealSizePerPixelW( float fRealPixelSizeW ) { m_fRealPixelSizeW = fRealPixelSizeW; }
float CxImageView::GetRealSizePerPixelH() const { return m_fRealPixelSizeH; }
void CxImageView::SetRealSizePerPixelH( float fRealPixelSizeH ) { m_fRealPixelSizeH = fRealPixelSizeH; }

ImageViewMode::ScreenMode CxImageView::GetScreenMode() const { return m_eScreenMode; }
void CxImageView::SetTracker( CRect& rcTrack ) { m_pInnerUI->m_RectTracker.m_rect = rcTrack; }
void CxImageView::SetTrackMode( BOOL bIsFixed ) { m_pInnerUI->m_bFixedTracker = bIsFixed; }

void CxImageView::SetMouseWheelMode( ImageViewMode::MouseWheelMode eMouseWheelMode ) { m_eMouseWheelMode = eMouseWheelMode; }
ImageViewMode::MouseWheelMode CxImageView::GetMouseWheelMode() const { return m_eMouseWheelMode; }

#ifdef _DEBUG
void CxImageView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CxImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG

void CxImageView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
}

void CxImageView::ShowScaleBar( BOOL bShow )
{
	m_bShowScaleBar = bShow;
}

void CxImageView::ShowDigitize( BOOL bShow )
{
	m_bShowDigitize = bShow;
}

void CxImageView::SetRegisterCallBack( UINT nIndexData, REGISTER_CALLBACK* pRegisterCB )
{
	m_fnOnDrawExt				= pRegisterCB->fnOnDrawExt;
	m_fnOnMeasure				= pRegisterCB->fnOnMeasure;
	m_fnOnConfirmTracker		= pRegisterCB->fnOnConfirmTracker;
	
	m_fnOnFireMouseEvent		= pRegisterCB->fnOnFireMouseEvent;
	m_fnOnEvent					= pRegisterCB->fnOnEvent;
	
	m_lpUsrDataOnDrawExt		= pRegisterCB->lpUsrData[0];
	m_lpUsrDataOnMeasure		= pRegisterCB->lpUsrData[1];
	m_lpUsrDataOnConfirmTracker	= pRegisterCB->lpUsrData[2];

	m_lpUsrDataOnFireMouseEvent	= pRegisterCB->lpUsrData[3];
	m_lpUsrDataOnEvent			= pRegisterCB->lpUsrData[4];

	m_nIndexData				= nIndexData;
}

void CxImageView::ShowScrollBar( BOOL bShow )
{
	//m_bShowScrollBar = bShow;
}

BOOL CxImageView::IsShowScrollBar() const
{
	//return m_bShowScrollBar;
	return FALSE;
}

void CxImageView::ShowDrawElapsedTime( BOOL bShow )
{
	m_bShowDrawElapsedTime = bShow;
}

void CxImageView::UseAutoFocus( BOOL bUse )
{
	m_bUseAutoFocus = bUse;
}

BOOL CxImageView::IsUseAutoFocus() const
{
	return m_bUseAutoFocus;
}

void CxImageView::EnableMouseControl( BOOL bEnable )
{
	m_bEnableMouseControl = bEnable;
}

BOOL CxImageView::IsEnableMouseControl() const
{
	return m_bEnableMouseControl;
}

void CxImageView::UpdateRenderer( CxImageObject* pImageObject )
{
	if ( !pImageObject || !pImageObject->IsValid() || pImageObject->GetBpp() == 0 )
		return;
	if ( m_nBitCnt == pImageObject->GetBpp() )
		return;

	CRect rcClient;
	GetClientRect( &rcClient );
	
	// initialize renderer
	InitGraphics( m_pDirectDIB, rcClient.Width(), rcClient.Height(), pImageObject->GetBpp() );

	m_pDrawDIB->SetDevice( m_pRenderer );

	m_MemDC.FillSolidRect( 0, 0, m_nWidth, m_nHeight, m_dwBackgroundColor );
}

BOOL CxImageView::SetImageObject( CxImageObject* pImgObj ) 
{ 
	// assign pointer of CxImageObject
	if ( m_pImageObject == pImgObj )
	{
		return FALSE;
	}
	m_pImageObject = pImgObj;
	//ASSERT( m_pImageObject );
	return TRUE;
}

void  CxImageView::SetZoomRatio( float fZoom, BOOL bInvalidate/*=TRUE*/ )
{
	// to limit zoom-range
	fZoom = CalcZoomValid( fZoom );

	if ( m_pImageObject != NULL )
	{
		m_sizeScrollRange.cx = int(m_pImageObject->GetWidth() * fZoom);
		m_sizeScrollRange.cy = int(m_pImageObject->GetHeight() * fZoom);
	}	

	if ( m_pInnerUI->m_bFixedTracker )
	{
		m_pInnerUI->m_RectTracker.m_rect = MousePosToImagePos( 
			m_pInnerUI->m_RectTracker.m_rect.left, 
			m_pInnerUI->m_RectTracker.m_rect.top, 
			m_pInnerUI->m_RectTracker.m_rect.right, 
			m_pInnerUI->m_RectTracker.m_rect.bottom );
	}

	m_fZoomRatio = fZoom;

	CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
	if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		if ( pWnd && IsWindow(pWnd->GetSafeHwnd()) )
		{
			pWnd->RedrawTitle();
		}
	}
	
	if ( m_pInnerUI->m_bFixedTracker )
	{
		//m_RectTracker.m_rect = MousePosToImagePos( m_RectTracker.m_rect.left, m_RectTracker.m_rect.top, m_RectTracker.m_rect.right, m_RectTracker.m_rect.bottom );
		m_pInnerUI->m_RectTracker.m_rect = ImagePosToScreenPos( 
			m_pInnerUI->m_RectTracker.m_rect.left, 
			m_pInnerUI->m_RectTracker.m_rect.top, 
			m_pInnerUI->m_RectTracker.m_rect.right, 
			m_pInnerUI->m_RectTracker.m_rect.bottom );
	}

	if ( bInvalidate ) Invalidate();
}

void CxImageView::DrawScreen( CDC* pDC )
{
	if (m_pImageObject)
	{
		if (m_pImageObject->IsNotifyFlag())
		{
			m_pImageObject->ClearNotifyFlag();
			UpdateRenderer( m_pImageObject );
			ZoomFit();
		}
	}

	//Clear();	// clear screen
	
	if ( !m_pImageObject || m_bLockUpdate || !m_pImageObject->IsValid() )
	{

#ifdef USE_MEMDC_IMAGE_VIEW
		m_MemDC.SetMapMode( MM_TEXT );
		m_MemDC.SetViewportOrg( 0, 0 );

		pDC->BitBlt( 0, 0, m_nWidth, m_nHeight, &m_MemDC, 0, 0, SRCCOPY );
#endif
		return;
	}
	
	CPoint ptScroll = GetDeviceScrollPosition();
	
	// move image to window's center
	m_nBodyOffsetX = 0;
	m_nBodyOffsetY = 0;
	int nRW = int(m_pImageObject->GetWidth()*m_fZoomRatio);
	int nRH = int(m_pImageObject->GetHeight()*m_fZoomRatio);
	if ( nRW < m_nWidth )
	{
		m_nBodyOffsetX = (m_nWidth-nRW);
		m_nBodyOffsetX >>= 1;
	}
	if ( nRH < m_nHeight )
	{
		m_nBodyOffsetY = (m_nHeight-nRH);
		m_nBodyOffsetY >>= 1;
	}
	
	m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
	m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
	
	if ( m_nSrcW > m_pImageObject->GetWidth() )
	{
		m_nSrcW = m_pImageObject->GetWidth();
	}
	if ( m_nSrcH > m_pImageObject->GetHeight() )
	{
		m_nSrcH = m_pImageObject->GetHeight();
	}
	
	// image rendering
	m_pDrawDIB->Draw( m_pImageObject, 
		m_nBodyOffsetX, m_nBodyOffsetY, 
		m_nWidth-m_nBodyOffsetX*2, m_nHeight-m_nBodyOffsetY*2, 
		m_nSrcX, m_nSrcY, 
		m_nSrcW, m_nSrcH, m_fZoomRatio );
	
	HDC hInnerDC = m_pRenderer->GetInnerDC();
	
	CDC* pInnerDC = CDC::FromHandle( hInnerDC );
	
	// digitized value text rendering
	if ( m_fZoomMax != 1.f && m_fZoomMax == m_fZoomRatio )
	{
		int nBlockSize = int(m_fZoomRatio);

		if ( m_bShowDigitize && m_eScreenMode != ImageViewMode::ScreenModePanning && (m_pImageObject->GetChannel() == 1) )
			DrawDigitizedValues( pInnerDC, nBlockSize, m_nBodyOffsetX, m_nBodyOffsetY, m_nSrcX, m_nSrcY, m_nSrcW, m_nSrcH );
	}
	
	m_pRenderer->ReleaseInnerDC();

	// finish
	CRect rcEnd;
	rcEnd.left = m_nBodyOffsetX;
	rcEnd.top = m_nBodyOffsetY;
	rcEnd.right = m_nWidth-m_nBodyOffsetX;
	rcEnd.bottom = m_nHeight-m_nBodyOffsetY;
	EndDraw( rcEnd );
	
	
	ptScroll = GetDeviceScrollPosition();
	
// 	CDC* pDC = GetDC();
#ifdef USE_MEMDC_IMAGE_VIEW
	CDC* pGDC = &m_MemDC;
#else
	CDC* pGDC = pDC;
#endif

	OnPrepareDC( pGDC );

	// scale-bar rendering
	if ( m_bShowScaleBar )
	{
		CRect rcScaleBar(10, m_nHeight-30, 210, m_nHeight-10);
		DrawScaleMark( pGDC->GetSafeHdc(), rcScaleBar, m_fZoomRatio/m_fRealPixelSizeW * 1000000, NULL );
	}

	if ( m_bShowDrawElapsedTime )
	{
		CRect rcBlock(m_nWidth-100, m_nHeight-50, m_nWidth, m_nHeight);
		DrawElapsedTime( pGDC->GetSafeHdc(), rcBlock );
	}
	
	m_GraphicObject.Draw( pGDC->GetSafeHdc() );
	
	if ( m_fnOnDrawExt )
	{
		(*m_fnOnDrawExt)( this, pGDC, m_nIndexData, m_lpUsrDataOnDrawExt );
	}
	else
	{
		OnDrawExt( this, pGDC );
	}
	
	if ( m_eScreenMode == ImageViewMode::ScreenModeMeasure )
	{
		DrawMeasure( pGDC );
	}
	
	if ( m_eScreenMode == ImageViewMode::ScreenModeTracker )
	{
		m_pInnerUI->m_RectTracker.Draw( pGDC );
	}

#ifdef USE_MEMDC_IMAGE_VIEW
	m_MemDC.SetMapMode( MM_TEXT );
	m_MemDC.SetViewportOrg( 0, 0 );

	pDC->BitBlt( 0, 0, m_nWidth, m_nHeight, &m_MemDC, 0, 0, SRCCOPY );
#endif
}

void CxImageView::OnDraw(CDC* pDC)
{
	CxStopWatch StopWatch;
	StopWatch.Begin();
	DrawScreen( pDC );
	StopWatch.Stop();
	m_dDrawElapsedTime = StopWatch.GetElapsedTime();
}

void CxImageView::DrawMeasure( CDC* pDC )
{
	CLineTracker::CLineItem* pLine;
	
	pLine = m_pInnerUI->m_LineList.GetFirst();
	
	CPoint ptStart;
	CPoint ptEnd;
	
	CString strDistance;
	
	while ( pLine ) 
	{
		ptStart.x = pLine->m_rcPoints.left;
		ptStart.y = pLine->m_rcPoints.top;
		
		ptEnd.x = pLine->m_rcPoints.right;
		ptEnd.y = pLine->m_rcPoints.bottom;

		CRect rcThisView;
		GetClientRect(rcThisView);

		if ( !rcThisView.PtInRect(ptStart) && !rcThisView.PtInRect(ptEnd) )
			return;

		int nDx = abs(pLine->m_rcPoints.right-pLine->m_rcPoints.left);
		int nDy = abs(pLine->m_rcPoints.bottom-pLine->m_rcPoints.top);
		
		float fDistanceX = float(nDx);
		float fDistanceY = float(nDy);
		fDistanceX *= m_fRealPixelSizeW / m_fZoomRatio;
		fDistanceY *= m_fRealPixelSizeH / m_fZoomRatio;
		float fDistance = sqrt( fDistanceX * fDistanceX + fDistanceY * fDistanceY );
	
		m_pInnerUI->m_MeasureArrowDrawer.Draw( pDC->GetSafeHdc(), ptStart, ptEnd, CxArrowDrawer::AT_BOTH );

		CPoint ptLineCenter = pLine->m_rcPoints.CenterPoint();
		
		strDistance.Format( _T("X: %.1fum(%dpx)\nY: %.1fum(%dpx)\nD: %.1fum"), fDistanceX, (int)(nDx/m_fZoomRatio+.5f), fDistanceY, (int)(nDy/m_fZoomRatio+.5f), fDistance );
		
		CRect rcCalcText(0, 0, 0, 0);
		pDC->DrawText(strDistance, rcCalcText, DT_CALCRECT|DT_LEFT);

		rcThisView.left += rcCalcText.Width() / 2; rcThisView.top += rcCalcText.Height() / 2;
		rcThisView.right -= rcCalcText.Width() / 2; rcThisView.bottom -= rcCalcText.Height() / 2;

		if (!rcThisView.PtInRect(ptLineCenter))
		{
			if (ptLineCenter.x < rcThisView.left)
				ptLineCenter.x = rcThisView.left;
			if (ptLineCenter.y < rcThisView.top)
				ptLineCenter.y = rcThisView.top;
			if (ptLineCenter.x > rcThisView.right)
				ptLineCenter.x = rcThisView.right;
			if (ptLineCenter.y > rcThisView.bottom)
				ptLineCenter.y = rcThisView.bottom;
		}

		CRect rcText( ptLineCenter.x - rcCalcText.Width() / 2, ptLineCenter.y - rcCalcText.Height() / 2, ptLineCenter.x, ptLineCenter.y );
		m_pInnerUI->m_MeasureArrowDrawer.DrawArrowText( pDC->GetSafeHdc(), rcText, (LPCTSTR)strDistance);

		pLine = pLine->m_pNext;
	}
	
	m_pInnerUI->m_LineTracker.Draw( pDC, CPoint(0,0) );
}

void CxImageView::DrawDigitizedValues( CDC* pDC, int nBlockSize, int nOffsetX, int nOffsetY, int nSrcX, int nSrcY, int nSrcW, int nSrcH )
{
	CString strLevel;

	DWORD dwOldTextColor = pDC->SetTextColor( RGB(0xff, 0xff, 0xff) );
	int nOldBkMode = pDC->SetBkMode( TRANSPARENT );

	int nT = (nSrcY+nSrcH)+1;
	int nW = (nSrcX+nSrcW)+1;

	CRect rcBlock(nOffsetX, nOffsetY, nOffsetX+nBlockSize, nOffsetY+nBlockSize);
	pDC->SetTextColor( RGB(0xff, 0xff, 0xff) );
	int nLevel;

	for ( int i=nSrcY ; i<nT ; i++ )
	{
		for ( int j=nSrcX ; j<nW ; j++ )
		{
			nLevel = m_pImageObject->GetPixelLevel( j, i );
			strLevel.Format( _T("%d"), nLevel );
			rcBlock.left--; rcBlock.right--;
			pDC->DrawText( strLevel, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
			rcBlock.left+=2; rcBlock.right+=2;
			pDC->DrawText( strLevel, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
			rcBlock.left--; rcBlock.right--;
			rcBlock.top--; rcBlock.bottom--;
			pDC->DrawText( strLevel, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
			rcBlock.top+=2; rcBlock.bottom+=2;
			pDC->DrawText( strLevel, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
			rcBlock.top--; rcBlock.bottom--;
			rcBlock.left += nBlockSize; rcBlock.right += nBlockSize;
		}
		rcBlock.left = nOffsetX;
		rcBlock.right = nOffsetX+nBlockSize;
		rcBlock.top += nBlockSize; rcBlock.bottom += nBlockSize;
	}

	rcBlock = CRect(nOffsetX, nOffsetY, nOffsetX+nBlockSize, nOffsetY+nBlockSize);
	pDC->SetTextColor( RGB(0x00, 0x00, 0x00) );
	for ( int i=nSrcY ; i<nT ; i++ )
	{
		for ( int j=nSrcX ; j<nW ; j++ )
		{
			nLevel = m_pImageObject->GetPixelLevel( j, i );
			strLevel.Format( _T("%d"), nLevel );

			pDC->DrawText( strLevel, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
			rcBlock.left += nBlockSize; rcBlock.right += nBlockSize;
		}
		rcBlock.left = nOffsetX;
		rcBlock.right = nOffsetX+nBlockSize;
		rcBlock.top += nBlockSize; rcBlock.bottom += nBlockSize;
	}

	pDC->SetTextColor( dwOldTextColor );
	pDC->SetBkMode( nOldBkMode );

}

BOOL CxImageView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style = cs.style & ~WS_BORDER;
	//cs.dwExStyle |= WS_EX_STATICEDGE;
	
	return CView::PreCreateWindow(cs);
}

int CxImageView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message) 
{
	return MA_ACTIVATE;	
	return CView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CxImageView::OnClose()
{
	CView::OnClose();
}

void CxImageView::OnDestroy() 
{
	CView::OnDestroy();
}

void CxImageView::OnNcDestroy() 
{
	CView::OnNcDestroy();
}

LRESULT CxImageView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message == WM_CLOSE || message == WM_DESTROY )
	{
		BOOL bIsWindow = IsWindow( GetSafeHwnd() );
		ExitGraphics(); 
	}
	return CView::WindowProc(message, wParam, lParam);
}


BOOL CxImageView::InitGraphics( CxRender2D* pRenderer, int nWidth, int nHeight, int nBitCnt )
{
	m_pRenderer = pRenderer; // &m_DirectDraw; //

	ExitGraphics();

	m_pDC = GetDC();
	m_pRenderer->SetDims( nWidth, nHeight );

#ifndef USE_MEMDC_IMAGE_VIEW
	HDC hDC = m_pDC->GetSafeHdc();
#else
	m_pBitmap = new CBitmap;
	ASSERT( m_pBitmap );

	m_MemDC.CreateCompatibleDC( m_pDC );
	VERIFY( m_pBitmap->CreateCompatibleBitmap( m_pDC, nWidth, nHeight) );

	m_pOldBitmap = m_MemDC.SelectObject( m_pBitmap );

	HDC hDC = m_MemDC.GetSafeHdc();
#endif

	m_pRenderer->SetDC( hDC );

	m_nWidth = nWidth;
	m_nHeight = nHeight;
	m_nBitCnt = nBitCnt;

	if ( !m_pRenderer->Init( m_nBitCnt ) )
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CxImageView::ExitGraphics()
{
	if ( m_pRenderer != NULL )
		m_pRenderer->Shutdown();

#ifdef USE_MEMDC_IMAGE_VIEW
	if ( m_pOldBitmap )
	{
		m_MemDC.SelectObject( m_pOldBitmap );
		m_pOldBitmap = NULL;
	}

	if ( m_MemDC.GetSafeHdc() )
		m_MemDC.DeleteDC();

	if ( m_pBitmap != NULL ) 
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
#endif

	if ( m_pDC != NULL )
	{
		ReleaseDC( m_pDC );
		m_pDC = NULL;
	}

	return TRUE;
}

void CxImageView::EndDraw( RECT rcEnd )
{
	ASSERT( m_pRenderer != NULL );
	m_pRenderer->EndDraw(rcEnd);
}

void CxImageView::EndDraw()
{
	ASSERT( m_pRenderer != NULL );
	m_pRenderer->EndDraw();
}

void CxImageView::SetClearColor( BYTE c )
{
	m_cClearColor = c;
}

void CxImageView::SetBackgroundColor( DWORD dwColor )
{
	m_dwBackgroundColor = dwColor;
}

void CxImageView::SetPopupMenuColor( DWORD dwBackgroundColor, DWORD dwTitleTextColor )
{
	m_pInnerUI->m_WheelPopupMenu.SetSideBarColor(dwBackgroundColor, dwBackgroundColor);
	m_pInnerUI->m_WheelPopupMenu.SetSideBarTextColor(dwTitleTextColor);
	m_pInnerUI->m_ROIPopupMenu.SetSideBarColor(dwBackgroundColor, dwBackgroundColor);
	m_pInnerUI->m_ROIPopupMenu.SetSideBarTextColor(dwTitleTextColor);
}

void CxImageView::Clear()
{
	if ( ! m_pRenderer->IsInit() ) return;
	if ( m_bLockUpdate ) return;
	BYTE* pBuffer = NULL;
	m_pRenderer->Lock();
	BYTE* pOffBuffer = m_pRenderer->GetOffBuffer(m_nWidthBytes);
	if ( !pOffBuffer )
	{
		m_pRenderer->Unlock();
		return;
	}
	pBuffer = ( m_nWidthBytes < 0 ) ? pOffBuffer + ( m_nHeight - 1 ) * m_nWidthBytes : pOffBuffer;
	int nWidthBytes = m_nWidthBytes < 0 ? -m_nWidthBytes : m_nWidthBytes;

	memset( pBuffer, m_cClearColor, m_nHeight * nWidthBytes );	// clear color: RGB(128, 128, 128) or Index(128)

	m_pRenderer->Unlock();
}

void CxImageView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

	if ( cx <= 0 || cy <= 0 ) return;

	CRect rcClient;
	GetClientRect( rcClient );
	
	int nWidth = rcClient.Width();
	int nHeight = rcClient.Height();

	if ( m_pRenderer != NULL )
	{
		InitGraphics( m_pRenderer, nWidth, nHeight, m_nBitCnt );
	}
}

BOOL CxImageView::OnEraseBkgnd(CDC* pDC) 
{
	if (m_MemDC.GetSafeHdc())
		m_MemDC.FillSolidRect( 0, 0, m_nWidth, m_nHeight, m_dwBackgroundColor );
	return TRUE;
}

void CxImageView::ZoomTo( CPoint ptScreen, float fZoom, BOOL bSyncControl/*=FALSE*/ )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) return;
	
	CRect rcClient;
	GetClientRect( &rcClient );
	CPoint ptCenter = rcClient.CenterPoint();
	SetRedraw( FALSE );
	CPoint ptImage = MousePosToImagePos( ptScreen.x, ptScreen.y );

	SetZoomRatio( fZoom, FALSE );
	CPoint ptOld = MousePosToImagePos( ptScreen.x, ptScreen.y );

	CPoint ptImageCenter = MousePosToImagePos( ptCenter.x, ptCenter.y );

	ptImageCenter.x += (ptImage.x-ptOld.x);
	ptImageCenter.y += (ptImage.y-ptOld.y);

	MoveTo( ptImageCenter, TRUE );

	if ( !bSyncControl )
	{
		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			CPoint ptScroll = GetDeviceScrollPosition();		
			m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
			m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
			
			pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
		}
	}

	SetRedraw( TRUE );
	
	Invalidate();
}

void CxImageView::ImageZoomTo( CPoint ptImage, float fZoom, BOOL bSyncControl/*=FALSE*/ )
{
//	SetRedraw( FALSE );
	if ( fZoom != 0.f )
		SetZoomRatio( fZoom, FALSE );

	CPoint ptScroll = GetDeviceScrollPosition();

	if ( !m_pImageObject )
		return;
	// move image to window's center
	m_nBodyOffsetX = 0;
	m_nBodyOffsetY = 0;
	int nRW = int(m_pImageObject->GetWidth()*m_fZoomRatio);
	int nRH = int(m_pImageObject->GetHeight()*m_fZoomRatio);
	if ( nRW < m_nWidth )
	{
		m_nBodyOffsetX = (m_nWidth-nRW);
		m_nBodyOffsetX >>= 1;
	}
	if ( nRH < m_nHeight )
	{
		m_nBodyOffsetY = (m_nHeight-nRH);
		m_nBodyOffsetY >>= 1;
	}

	m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
	m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);

	MoveTo( ptImage, bSyncControl );

//	SetRedraw( TRUE );
	Invalidate();
}

void CxImageView::MoveTo( CPoint ptImage, BOOL bSyncControl/*=FALSE*/ )
{
	int nHalfWidth = m_nWidth >> 1;
	int nHalfHeight = m_nHeight >> 1;
	CPoint ptDevice = ImagePosToScreenPos( ptImage.x, ptImage.y );
//	TRACE( "%d, %d\r\n", ptDevice.x, ptDevice.y );

	CSize szScroll = m_sizeScrollRange;
	
	int nScrollX = ptDevice.x - nHalfWidth;
	int nScrollY = ptDevice.y - nHalfHeight;

	int nAvaliableScrollX = szScroll.cx - m_nWidth;
	int nAvaliableScrollY = szScroll.cy - m_nHeight;

	nScrollX > nAvaliableScrollX ? nScrollX = nAvaliableScrollX : 0;
	nScrollX < 0 ? nScrollX = 0 : 0;
	nScrollY > nAvaliableScrollY ? nScrollY = nAvaliableScrollY : 0;
	nScrollY < 0 ? nScrollY = 0 : 0;

	m_ptScrollPos.x = nScrollX;
	m_ptScrollPos.y = nScrollY;

 	if ( !bSyncControl )
 	{
 		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
 		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
 		{
 			pWnd->SyncDevContext( this, ptImage, TRUE );
 		}
 	}
}

void CxImageView::ZoomNot()
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) 
		return;

	SetZoomRatio( 1.f );

	CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
	if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CPoint ptScroll = GetDeviceScrollPosition();		
		m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
		m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
		pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
	}
}

void CxImageView::OnZoomFit( BOOL bCalcScrollBar /*=TRUE*/ )
{
	SetRedraw( FALSE );

	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) 
	{
		return;
	}

	RecalcZoomRatio( bCalcScrollBar );

	SetRedraw( TRUE );

	SetZoomRatio( m_fZoomMin );	
}

void CxImageView::ZoomFit( BOOL bCalcScrollBar /*= TRUE*/ )
{
	SetRedraw( FALSE );

	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) 
	{
		return;
	}

	RecalcZoomRatio( bCalcScrollBar );

	SetRedraw( TRUE );

	SetZoomRatio( m_fZoomFit );

	CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
	if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CPoint ptScroll = GetDeviceScrollPosition();		
		m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
		m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
		pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
	}
}

void CxImageView::RecalcZoomRatio( BOOL bCalcScrollBar /*= TRUE*/ )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) return;
	
	float fTan = (float)m_pImageObject->GetHeight() / m_pImageObject->GetWidth();
	float fRatio;
	if ( fTan < (float)(m_nHeight)/(m_nWidth) )
	{
		fRatio = (float)(m_nWidth) / m_pImageObject->GetWidth();
	}
	else
	{
		fRatio = (float)(m_nHeight) / m_pImageObject->GetHeight();
	}

	m_fZoomFit = fRatio;
	if ( fRatio > 1.f ) fRatio = 1.f;
	m_fZoomMin = fRatio;

	m_fZoomMax = 30.f;
	if (m_pImageObject->GetChannel() == 1 &&
		m_pImageObject->GetDepth() == 16)
		m_fZoomMax = 50.f;

	//TRACE( _T("RecalcZoomRatio: %f/%f\n"), m_fZoomMin, m_fZoomMax );
}

void CxImageView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	if (!m_bEnableMouseControl)
		return;

	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_LBUTTONDBLCLK, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_LBUTTONDBLCLK, point ) )
			return;
	}

	float fZoom;
	switch ( m_eScreenMode )
	{
	case ImageViewMode::ScreenModeZoomInOut:
	case ImageViewMode::ScreenModeZoomIn:
		if ( m_fZoomRatio == m_fZoomMax )
			return;
		fZoom = m_fZoomRatio * 2.f;
		ZoomTo( point, fZoom );
		fZoom = m_fZoomRatio * 2.f;
		ZoomTo( point, fZoom );
		break;
	case ImageViewMode::ScreenModeZoomOut:
		if ( m_fZoomRatio == m_fZoomMin )
			return;
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		break;
	}

	CView::OnLButtonDblClk(nFlags, point);
}

void CxImageView::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
	if (!m_bEnableMouseControl)
		return;

	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_RBUTTONDBLCLK, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_RBUTTONDBLCLK, point ) )
			return;
	}

	if ( ::GetCapture() != m_hWnd ) ::SetCapture( m_hWnd );

	m_ptViewLastMouse = point;

	float fZoom;

	switch ( m_eScreenMode )
	{
	case ImageViewMode::ScreenModeZoomInOut:
		if ( m_fZoomRatio == m_fZoomMin )
			return;
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		break;
	}
	
	CView::OnRButtonDblClk(nFlags, point);
}

void CxImageView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_LBUTTONDOWN, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_LBUTTONDOWN, point ) )
			return;
	}

	if (!m_bEnableMouseControl)
		return;

	if ( GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CWnd* pParentWnd = GetParent()->GetParent();
		if ( pParentWnd && pParentWnd->IsTopParentActive() )
		{
			GetParent()->SetFocus();
		}
	}

	if ( m_eScreenMode == ImageViewMode::ScreenModeMeasure )
	{
		if ( !m_pInnerUI->m_LineTracker.OnLButtonDown(this, nFlags, point, &m_pInnerUI->m_LineList) ) 
		{
			// If not lines were hit then start a local CLineTracker to rubber band a new line
			CLineTracker tracker;
			
			if ( tracker.TrackRubberLine(this, point) ) 
			{
				// Make sure we have a line that is long enough
				if ( (abs(tracker.m_points.Width()) > 20) || (abs(tracker.m_points.Height()) > 20) ) 
				{ 
					m_pInnerUI->m_LineList.RemoveAll();
					// Add the line to our (very simple) linked list
					CLineTracker::CLineItem* pLine = new CLineTracker::CLineItem();
					
					pLine->m_rcPoints.SetRect(	tracker.m_points.left,
												tracker.m_points.top,
												tracker.m_points.right,
												tracker.m_points.bottom);
					m_pInnerUI->m_LineList.Add(pLine);

					float fDistanceX = float(abs(tracker.m_points.right-tracker.m_points.left));
					float fDistanceY = float(abs(tracker.m_points.bottom-tracker.m_points.top));
					fDistanceX *= m_fRealPixelSizeW / m_fZoomRatio;
					fDistanceY *= m_fRealPixelSizeH / m_fZoomRatio;

					if ( m_fnOnMeasure )
					{
						(*m_fnOnMeasure)( fDistanceX, fDistanceY, m_nIndexData, m_lpUsrDataOnMeasure );
					}
					else
					{
						OnMeasure( fDistanceX, fDistanceY );
					}
					
				}
			}
		}
		else
		{
			float fDistanceX = float(abs(m_pInnerUI->m_LineTracker.m_points.right-m_pInnerUI->m_LineTracker.m_points.left));
			float fDistanceY = float(abs(m_pInnerUI->m_LineTracker.m_points.bottom-m_pInnerUI->m_LineTracker.m_points.top));
			fDistanceX *= m_fRealPixelSizeW / m_fZoomRatio;
			fDistanceY *= m_fRealPixelSizeH / m_fZoomRatio;
			if ( m_fnOnMeasure )
			{
				(*m_fnOnMeasure)( fDistanceX, fDistanceY, m_nIndexData, m_lpUsrDataOnMeasure );
			}
			else
			{
				OnMeasure( fDistanceX, fDistanceY );
			}
		}

		Invalidate();
		CView::OnLButtonDown(nFlags, point);
		return;
	}

	if ( m_eScreenMode == ImageViewMode::ScreenModeTracker )
	{
		if ( m_pInnerUI->m_RectTracker.HitTest(point) == CRectTracker::hitNothing )
		{
			if ( m_pInnerUI->m_RectTracker.TrackRubberBand( this, point, TRUE ) )
			{
			}
		}
		else
		{
			if ( m_pInnerUI->m_RectTracker.Track( this, point ) )
			{
			}
		}

		Invalidate();
		return;
	}
	
	if ( ::GetCapture() != m_hWnd ) ::SetCapture( m_hWnd );

	m_ptViewLastMouse = point;

	float fZoom;

	switch ( m_eScreenMode )
	{
	case ImageViewMode::ScreenModeZoomInOut:
	case ImageViewMode::ScreenModeZoomIn:
		if ( m_fZoomRatio == m_fZoomMax )
			return;
		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventZoomIn, m_nIndexData, m_lpUsrDataOnEvent );
		else
			OnEvent( ImageViewEvent::ActionEventZoomIn );
		fZoom = m_fZoomRatio * 2.f;
		ZoomTo( point, fZoom );
		break;
	case ImageViewMode::ScreenModeZoomOut:
		if ( m_fZoomRatio == m_fZoomMin )
			return;
		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventZoomOut, m_nIndexData, m_lpUsrDataOnEvent );
		else
			OnEvent( ImageViewEvent::ActionEventZoomOut );
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		break;
	}
	
	CView::OnLButtonDown(nFlags, point);
}

void CxImageView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_LBUTTONUP, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_LBUTTONUP, point ) )
			return;
	}

	if (!m_bEnableMouseControl)
		return;

	if ( m_eScreenMode == ImageViewMode::ScreenModeMeasure || m_eScreenMode == ImageViewMode::ScreenModeTracker )
	{
		CView::OnLButtonUp(nFlags, point);
		return;
	}

	if ( ::GetCapture() == m_hWnd )
		::ReleaseCapture();

	m_ptViewLastMouse = point;
	m_ptViewLastMouse.x = m_ptViewLastMouse.y = -1;

	Invalidate();
	
	CView::OnLButtonUp(nFlags, point);
}

void CxImageView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_RBUTTONDOWN, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_RBUTTONDOWN, point ) )
			return;
	}

	if (!m_bEnableMouseControl)
		return;

	if ( GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CWnd* pParentWnd = GetParent()->GetParent();
		if ( pParentWnd && pParentWnd->IsTopParentActive() )
		{
			GetParent()->SetFocus();
		}
	}

	if ( ::GetCapture() != m_hWnd ) ::SetCapture( m_hWnd );

	m_ptViewLastMouse = point;

	float fZoom;

	switch ( m_eScreenMode )
	{
	case ImageViewMode::ScreenModeZoomInOut:
		if ( m_fZoomRatio == m_fZoomMin )
			return;
		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventZoomOut, m_nIndexData, m_lpUsrDataOnEvent );
		else
			OnEvent( ImageViewEvent::ActionEventZoomOut );
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( point, fZoom );
		break;
	}
	
	CView::OnRButtonDown(nFlags, point);
}

void CxImageView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_RBUTTONUP, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_RBUTTONUP, point ) )
			return;
	}

	if (!m_bEnableMouseControl)
		return;

	if ( ::GetCapture() == m_hWnd )
		::ReleaseCapture();

	m_ptViewLastMouse = point;
	m_ptViewLastMouse.x = m_ptViewLastMouse.y = -1;
	
	CView::OnRButtonUp(nFlags, point);
}

void CxImageView::MousePan( CPoint& point )
{
	CPoint ptScroll = GetDeviceScrollPosition();
	
	CPoint ptMove = point;
	ptMove -= m_ptViewLastMouse;
	
	ptScroll -= ptMove;
	BOOL bUpdateHorz = FALSE;
	BOOL bUpdateVert = FALSE;
	if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth() )
	{
		if (m_ptScrollPos.x != ptScroll.x)
		{
			bUpdateHorz = TRUE;
		}
		m_ptScrollPos.x = ptScroll.x;
	}
	if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
	{
		if (m_ptScrollPos.y != ptScroll.y)
		{
			bUpdateVert = TRUE;
		}
		m_ptScrollPos.y = ptScroll.y;
	}
	
	BOOL bLockMouse = FALSE;
	if ( ptScroll.x > m_sizeScrollRange.cx || ptScroll.x < 0 )
	{
		point.x = m_ptViewLastMouse.x;
		bLockMouse = TRUE;
	}
	if ( ptScroll.y > m_sizeScrollRange.cy || ptScroll.y < 0 )
	{
		point.y = m_ptViewLastMouse.y;
		bLockMouse = TRUE;
	}
	
	if ( bLockMouse && m_bEnableLockMouse )
	{
		ClientToScreen( &point );
		SetCursorPos( point.x, point.y );
		
	}
	else
	{
		m_ptViewLastMouse = point;
	}
	
	if (bUpdateVert || bUpdateHorz)
	{
		Invalidate();
	}
	m_eScreenMode = ImageViewMode::ScreenModePanning;
	SetCursor( m_hCursorPan );
}

ImageViewMode::ScreenMode CxImageView::SetScreenMode( ImageViewMode::ScreenMode _ScreenMode ) 
{ 
	m_pInnerUI->m_RectTracker.m_rect = CRect(-100,-100,-100,-100);
	ImageViewMode::ScreenMode eOldMode = m_eScreenMode; 
	m_eScreenMode = _ScreenMode; 
	if ( m_eScreenMode == ImageViewMode::ScreenModeMeasure )
	{
		if ( GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
			CString strText;
			strText.LoadString(GetResourceHandle(), IDS_MEASURE_MODE);
			pWnd->OnStatusText( strText );
		}
	}
	if ( m_eScreenMode == ImageViewMode::ScreenModeTracker )
	{
		if ( GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
			CString strText;
			strText.LoadString(GetResourceHandle(), IDS_TRACKER_MODE);
			pWnd->OnStatusText( strText );
		}

		SetCursor( m_hCursorTracker );
	}
	if ((eOldMode == ImageViewMode::ScreenModeMeasure) || (eOldMode == ImageViewMode::ScreenModeTracker))
	{
		if (m_eScreenMode != eOldMode)
		{
			Invalidate();
		}
	}
	return eOldMode; 
}

void CxImageView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CRect rc;
	GetClientRect( rc );
	
	if ( !m_bMouseOverCheck && rc.PtInRect(point) )
	{
		if ( m_fnOnFireMouseEvent )
		{
			(*m_fnOnFireMouseEvent)( WM_MOUSEENTER, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent );
		}
		// MOUSE ENTER EVENT
		m_bMouseOverCheck = TRUE;
		SetTimer( 1, 10, 0 );
	}
	
	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_MOUSEMOVE, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_MOUSEMOVE, point ) )
			return;
	}

	if (!m_bEnableMouseControl)
		return;

	if ( m_bUseAutoFocus && GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CWnd* pParentWnd = GetParent()->GetParent();
		if ( pParentWnd && pParentWnd->IsTopParentActive() )
		{
			GetParent()->SetFocus();
		}
	}

//	TRACE( "ScreenMode: %d\r\n", m_eScreenMode );

	if ( m_eScreenMode == ImageViewMode::ScreenModeMeasure )
	{
		CView::OnMouseMove(nFlags, point);
		return;
	}

	if ( ((MK_LBUTTON & nFlags) != 0)  && ((m_eScreenMode==ImageViewMode::ScreenModePanning)||(m_eScreenMode==ImageViewMode::ScreenModeNone)) )
	{
		if ( m_ptViewLastMouse.x == -1 &&  m_ptViewLastMouse.y == -1 ) 
		{
			CView::OnMouseMove(nFlags, point);
			return;
		}

		if ( m_eScreenMode == ImageViewMode::ScreenModeNone )
		{
			if ( m_fnOnEvent )
				(*m_fnOnEvent)( ImageViewEvent::ActionEventPanStart, m_nIndexData, m_lpUsrDataOnEvent );
			else
				OnEvent( ImageViewEvent::ActionEventPanStart );
		}

		MousePan( point );

		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			CPoint ptImage = MousePosToImagePos( point.x, point.y );
			pWnd->SyncDevContext( this, ptImage, TRUE );
		}

		return;
	}

	if ( m_eScreenMode == ImageViewMode::ScreenModePanning )
	{
		if ( m_fnOnEvent )
			(*m_fnOnEvent)( ImageViewEvent::ActionEventPanEnd, m_nIndexData, m_lpUsrDataOnEvent );
		else
			OnEvent( ImageViewEvent::ActionEventPanEnd );
		m_eScreenMode = ImageViewMode::ScreenModeNone;
	}

	if ( m_eScreenMode == ImageViewMode::ScreenModeNone )
		SetCursor( m_hCursorNormal );

	if ( !m_pImageObject ) return;

	BOOL bIsValidPos;
	CPoint ptImage = MousePosToImagePos( point.x, point.y, &bIsValidPos );

	if (bIsValidPos)
	{
		COLORREF dwColor;
		unsigned int nGV;
		switch ( m_pImageObject->GetBpp() )
		{
		case 8:
			nGV = m_pImageObject->GetPixelLevel( ptImage.x, ptImage.y );
			dwColor = RGB(nGV, nGV, nGV);
			break;
		case 16:
			nGV = m_pImageObject->GetPixelLevel( ptImage.x, ptImage.y );
			dwColor = m_pImageObject->GetPixelColor( ptImage.x, ptImage.y );
			break;
		case 24:
		case 32:
			nGV = 0;
			dwColor = m_pImageObject->GetPixelColor( ptImage.x, ptImage.y );
			break;
		}

		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			pWnd->OnStatusInfo( ptImage.x, ptImage.y, dwColor, nGV, m_pImageObject->GetDepth(), m_pImageObject->GetChannel() );
			pWnd->SyncDevContext( this, ptImage, FALSE );
		}
	}
	else
	{
		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			pWnd->OnStatusText( _T(" ") );
			pWnd->SyncDevContext( this, ptImage, FALSE );
		}
	}

	CView::OnMouseMove(nFlags, point);
}

DPOINT CxImageView::ImagePosToScreenPos( double dX, double dY )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) { return CxDPoint(0.,0.); }

	CPoint ptScroll = GetDeviceScrollPosition();
	
	double dMouseX = ( (dX-m_nSrcX) * m_fZoomRatio );
	dMouseX += m_nBodyOffsetX;
	dMouseX += ptScroll.x;
	double dMouseY = ( (dY-m_nSrcY) * m_fZoomRatio );
	dMouseY += m_nBodyOffsetY;
	dMouseY += ptScroll.y;

	return CxDPoint( dMouseX, dMouseY );
}

DRECT CxImageView::ImagePosToScreenPos( double dLeft, double dTop, double dRight, double dBottom )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) { return CxDRect(-10.,-10.,-10.,-10.); }

	CPoint ptScroll = GetDeviceScrollPosition();
	
	dLeft = ((dLeft-m_nSrcX) * m_fZoomRatio);	
	dLeft += m_nBodyOffsetX;
	dLeft += ptScroll.x;
	dTop = ((dTop-m_nSrcY) * m_fZoomRatio);
	dTop += m_nBodyOffsetY;
	dTop += ptScroll.y;
	dRight = ((dRight-m_nSrcX) * m_fZoomRatio);	
	dRight += m_nBodyOffsetX;
	dRight += ptScroll.x;
	dBottom = ((dBottom-m_nSrcY) * m_fZoomRatio);
	dBottom += m_nBodyOffsetY;
	dBottom += ptScroll.y;

	return CxDRect( dLeft, dTop, dRight, dBottom );
}

RECT CxImageView::ImagePosToScreenPos( int nLeft, int nTop, int nRight, int nBottom )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) { return CRect(-10,-10,-10,-10); }

	CPoint ptScroll = GetDeviceScrollPosition();
	
	nLeft = int((nLeft-m_nSrcX) * m_fZoomRatio);	
	nLeft += m_nBodyOffsetX;
	nLeft += ptScroll.x;
	nTop = int((nTop-m_nSrcY) * m_fZoomRatio);
	nTop += m_nBodyOffsetY;
	nTop += ptScroll.y;
	nRight = int((nRight-m_nSrcX) * m_fZoomRatio);	
	nRight += m_nBodyOffsetX;
	nRight += ptScroll.x;
	nBottom = int((nBottom-m_nSrcY) * m_fZoomRatio);
	nBottom += m_nBodyOffsetY;
	nBottom += ptScroll.y;

	return CRect( nLeft, nTop, nRight, nBottom );
}

POINT CxImageView::ImagePosToScreenPos( int x, int y)
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) { return CPoint(0,0); }

	CPoint ptScroll = GetDeviceScrollPosition();
	
	int nMouseX = int((x-m_nSrcX) * m_fZoomRatio);	
	nMouseX += m_nBodyOffsetX;
	nMouseX += ptScroll.x;
	int nMouseY = int((y-m_nSrcY) * m_fZoomRatio);
	nMouseY += m_nBodyOffsetY;
	nMouseY += ptScroll.y;

	return CPoint( nMouseX, nMouseY );
}

POINT CxImageView::MousePosToImagePos( int nMouseX, int nMouseY, BOOL* pbIsValidPos/*=NULL*/ )
{	
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL)
	{
		if (pbIsValidPos != NULL) { *pbIsValidPos = FALSE; }
		return CPoint(0,0);
	}

	CPoint ptScroll = GetDeviceScrollPosition();

	m_nBodyOffsetX = 0;
	m_nBodyOffsetY = 0;
	int nRW = int(m_pImageObject->GetWidth()*m_fZoomRatio);
	int nRH = int(m_pImageObject->GetHeight()*m_fZoomRatio);
	if ( nRW < m_nWidth )
	{
		m_nBodyOffsetX = (m_nWidth-nRW);
		m_nBodyOffsetX >>= 1;
	}
	if ( nRH < m_nHeight )
	{
		m_nBodyOffsetY = (m_nHeight-nRH);
		m_nBodyOffsetY >>= 1;
	}

	m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
	m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
	
	nMouseX -= m_nBodyOffsetX;
	nMouseY -= m_nBodyOffsetY;

	float fRatio = 1.f/m_fZoomRatio;

	int nTH = int((nMouseY)*fRatio+m_nSrcY);
	int nTW = int((nMouseX)*fRatio+m_nSrcX);	
	
	ptScroll.x = nTW;
	ptScroll.y = nTH;

	BOOL bIsValidPos = TRUE;

	if ( ptScroll.x < 0 ) { ptScroll.x = 0; bIsValidPos = FALSE; }
	if ( ptScroll.y < 0 ) { ptScroll.y = 0; bIsValidPos = FALSE; }
	if ( ptScroll.x >= m_pImageObject->GetWidth() ) { ptScroll.x = m_pImageObject->GetWidth()-1; bIsValidPos = FALSE; }
	if ( ptScroll.y >= m_pImageObject->GetHeight() ) { ptScroll.y = m_pImageObject->GetHeight()-1; bIsValidPos = FALSE; }

	if (pbIsValidPos != NULL) { *pbIsValidPos = bIsValidPos; }

	return CPoint(ptScroll.x, ptScroll.y);
}

RECT CxImageView::MousePosToImagePos( int nMouseLeft, int nMouseTop, int nMouseRight, int nMouseBottom )
{	
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL) { return CRect(0,0,0,0); }

	CPoint ptScroll = GetDeviceScrollPosition();

	m_nBodyOffsetX = 0;
	m_nBodyOffsetY = 0;
	int nRW = int(m_pImageObject->GetWidth()*m_fZoomRatio);
	int nRH = int(m_pImageObject->GetHeight()*m_fZoomRatio);
	if ( nRW < m_nWidth )
	{
		m_nBodyOffsetX = (m_nWidth-nRW);
		m_nBodyOffsetX >>= 1;
	}
	if ( nRH < m_nHeight )
	{
		m_nBodyOffsetY = (m_nHeight-nRH);
		m_nBodyOffsetY >>= 1;
	}

	m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
	m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
	
	nMouseLeft -= m_nBodyOffsetX;
	nMouseRight -= m_nBodyOffsetX;
	nMouseTop -= m_nBodyOffsetY;
	nMouseBottom -= m_nBodyOffsetY;

	float fRatio = 1.f/m_fZoomRatio;

	int nTTop = int((nMouseTop)*fRatio+m_nSrcY);
	int nTLeft = int((nMouseLeft)*fRatio+m_nSrcX);
	int nTBottom = int((nMouseBottom)*fRatio+m_nSrcY);
	int nTRight = int((nMouseRight)*fRatio+m_nSrcX);
	
	if ( nTLeft < 0 ) nTLeft = 0; if ( nTTop < 0 ) nTTop = 0; if ( nTRight < 0 ) nTRight = 0; if ( nTBottom < 0 ) nTBottom = 0;

	if ( nTLeft >= m_pImageObject->GetWidth() ) nTLeft = m_pImageObject->GetWidth()-1;
	if ( nTRight >= m_pImageObject->GetWidth() ) nTRight = m_pImageObject->GetWidth()-1;
	if ( nTTop >= m_pImageObject->GetHeight() ) nTTop = m_pImageObject->GetHeight()-1;
	if ( nTBottom >= m_pImageObject->GetHeight() ) nTBottom = m_pImageObject->GetHeight()-1;

	return CRect(nTLeft, nTTop, nTRight, nTBottom);
}

POINT CxImageView::ScreenPosToOverlay( int x, int y )
{
	if ( m_pImageObject == NULL || m_pImageObject->GetImageBuffer() == NULL ) { return CPoint(0,0); }

	return CPoint(x, y);
}

BOOL CxImageView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (!m_bEnableMouseControl)
		return CView::OnSetCursor(pWnd, nHitTest, message);

	if ( m_pInnerUI->m_LineTracker.SetCursor(pWnd, nHitTest) )
		return TRUE;

	if ( pWnd == this && m_pInnerUI->m_RectTracker.SetCursor(this, nHitTest) )
		return TRUE;

	if ( nHitTest != HTCLIENT )
	{
		return CView::OnSetCursor(pWnd, nHitTest, message);
	}

	switch ( m_eScreenMode )
	{
	case ImageViewMode::ScreenModePanning:
		SetCursor( m_hCursorPan );
		break;
	case ImageViewMode::ScreenModeZoomIn:
		if ( m_fZoomRatio == m_fZoomMax )
		{
			SetCursor( m_hCursorZoomNot );
		}
		else
		{
			SetCursor( m_hCursorZoomIn );
		}
		break;
	case ImageViewMode::ScreenModeZoomOut:
		if ( m_fZoomRatio == m_fZoomMin )
		{
			SetCursor( m_hCursorZoomNot );
		}
		else
		{
			SetCursor( m_hCursorZoomOut );
		}
		break;
	case ImageViewMode::ScreenModeZoomInOut:
		if ( m_fZoomRatio == m_fZoomMax )
		{
			SetCursor( m_hCursorZoomOut );
		}
		else if ( m_fZoomRatio == m_fZoomMin )
		{
			SetCursor( m_hCursorZoomIn );
		}
		else
		{
			SetCursor( m_hCursorZoomInOut );
		}
		break;
	case ImageViewMode::ScreenModeTracker:
		SetCursor( m_hCursorTracker );
		break;
	default:
		SetCursor( m_hCursorNormal );
		break;
	}

	return TRUE;
}

void CxImageView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (!m_pImageObject || !m_pImageObject->IsValid())
		return;

	if (!m_bEnableMouseControl)
		return;

	CPoint ptScroll = GetDeviceScrollPosition();

	nRepCnt = UINT(nRepCnt*10 * m_fZoomRatio + .5);
	if ( nRepCnt < 1 ) nRepCnt = 1;

	SCROLLINFO scrlInfo;
	GetScrollInfo(SB_VERT, &scrlInfo);

	CPoint ptCenter;
	ptCenter.x = m_nWidth / 2;
	ptCenter.y = m_nHeight / 2;
	float fZoom;

	BOOL bZoom = FALSE;
	switch (nChar)
	{
	case VK_UP:
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = ptScroll.y-nRepCnt;	// TODO:: Limit Scroll Pos
		}
		break;
	case VK_DOWN:
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = ptScroll.y+nRepCnt;
		}
		break;
	case VK_LEFT:
		if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth() )
		{
			m_ptScrollPos.x = ptScroll.x-nRepCnt;
		}
		break;
	case VK_RIGHT:
		if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth() )
		{
			m_ptScrollPos.x = ptScroll.x+nRepCnt;
		}
		break;
	case VK_HOME:
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = 0;
		}
		break;
	case VK_END:
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = m_sizeScrollRange.cy;
		}
		break;
	case VK_PRIOR:		// page up
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = ptScroll.y-scrlInfo.nPage;
		}
		break;
	case VK_NEXT:		// page down
		if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
		{
			m_ptScrollPos.y = ptScroll.y+scrlInfo.nPage;
		}
		break;
	case VK_ADD:
	case 0xbb:
		if ( m_fZoomRatio == m_fZoomMax )
			return;
		fZoom = m_fZoomRatio * 2.f;
		ZoomTo( ptCenter, fZoom );
		bZoom = TRUE;
		break;
	case VK_SUBTRACT:
	case 0xbd:
		if ( m_fZoomRatio == m_fZoomMin )
			return;
		fZoom = m_fZoomRatio / 2.f;
		ZoomTo( ptCenter, fZoom );
		bZoom = TRUE;
		break;
	}

	if ( !bZoom )
	{
		CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
		if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
		{
			CPoint ptScroll = GetDeviceScrollPosition();		
			m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
			m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);

			pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
		}
	}

	Invalidate();
}

BOOL CxImageView::PreTranslateMessage(MSG* pMsg) 
{
	if (!m_pImageObject || !m_pImageObject->IsValid())
		return CView::PreTranslateMessage(pMsg);

	if (!m_bEnableMouseControl)
		return CView::PreTranslateMessage(pMsg);

	CPoint ptScroll = GetDeviceScrollPosition();

	switch (pMsg->message)
	{
	case WM_DESTROY:
		TRACE( "XX" );
		break;
	case WM_KEYDOWN:
		switch (pMsg->wParam)
		{
		case VK_UP:
			if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight())
			{
				m_ptScrollPos.y = ptScroll.y+1;
			}
			break;
		case VK_DOWN:
			if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight())
			{
				m_ptScrollPos.y = ptScroll.y-1;
			}
			break;
		case VK_LEFT:
			if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth())
			{
				m_ptScrollPos.x = ptScroll.x+1;
			}
			break;
		case VK_RIGHT:
			if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth())
			{
				m_ptScrollPos.x = ptScroll.x-1;
			}
			break;
		case VK_HOME:
			if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight())
			{
				m_ptScrollPos.y = 0;
			}
			break;
		case VK_END:
			if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight())
			{
				m_ptScrollPos.y = m_sizeScrollRange.cy;
			}
			break;
		}
		break;
	case WM_KEYUP:
		break;
	} 

	return CView::PreTranslateMessage(pMsg);
}

BOOL CxImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	if (!m_bEnableMouseControl)
		return FALSE;

	pt.x = (short)pt.x;
	pt.y = (short)pt.y;
	ScreenToClient( &pt );

	CRect rcClient;
	GetClientRect( rcClient );
	if ( !rcClient.PtInRect(pt) )
		return FALSE;

	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_MOUSEWHEEL, pt, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return FALSE;
	}
	else
	{
		if ( OnFireMouseEvent( WM_MOUSEWHEEL, pt ) )
			return FALSE;
	}

	switch ( m_eMouseWheelMode )
	{
	case ImageViewMode::MouseWheelModeZoom:
		{
			float fZoom;	
			if ( zDelta > 0 )
			{
				if ( m_fnOnEvent )
					(*m_fnOnEvent)( ImageViewEvent::ActionEventZoomIn, m_nIndexData, m_lpUsrDataOnEvent );
				else
					OnEvent( ImageViewEvent::ActionEventZoomIn );

				fZoom = m_fZoomRatio * 2.f;
				if ( m_fZoomRatio == m_fZoomMax ) return FALSE;
				ZoomTo( pt, fZoom );
			}
			else
			{
				if ( m_fnOnEvent )
					(*m_fnOnEvent)( ImageViewEvent::ActionEventZoomOut, m_nIndexData, m_lpUsrDataOnEvent );
				else
					OnEvent( ImageViewEvent::ActionEventZoomOut );

				fZoom = m_fZoomRatio / 2.f;
				if ( m_fZoomRatio == m_fZoomMin ) return FALSE;
				ZoomTo( pt, fZoom );
			}
		}
		break;
	case ImageViewMode::MouseWheelModeVerticalScroll:
		{
			if (m_pImageObject && m_pImageObject->IsValid())
			{
				CPoint ptScroll = GetDeviceScrollPosition();
				if ( m_sizeScrollRange.cy > m_pImageObject->GetHeight() )
				{
					m_ptScrollPos.y = ptScroll.y-zDelta;
				}
			}

			CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
			if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
			{
				CPoint ptScroll = GetDeviceScrollPosition();		
				m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
				m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
				pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
			}

			Invalidate();
		}
		break;
	case ImageViewMode::MouseWheelModeHorizontalScroll:
		{
			if (m_pImageObject && m_pImageObject->IsValid())
			{
				CPoint ptScroll = GetDeviceScrollPosition();
				if ( m_sizeScrollRange.cx > m_pImageObject->GetWidth() )
				{
					m_ptScrollPos.x = ptScroll.x+zDelta;
				}
			}

			CxImageViewCtrl* pWnd = (CxImageViewCtrl*)GetParent();
			if ( pWnd->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
			{
				CPoint ptScroll = GetDeviceScrollPosition();		
				m_nSrcX = int(ptScroll.x/m_fZoomRatio); m_nSrcY = int(ptScroll.y/m_fZoomRatio);
				m_nSrcW = int(m_nWidth/m_fZoomRatio);   m_nSrcH = int(m_nHeight/m_fZoomRatio);
				pWnd->SyncDevContext( this, CPoint(m_nSrcX+m_nSrcW/2, m_nSrcY+m_nSrcH/2), TRUE );
			}

			Invalidate();
		}
		break;
	}
	
	return FALSE;
	//return CScrollView::OnMouseWheel(nFlags, zDelta, pt);
}

void CxImageView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bEnableMouseControl)
		return;

	if ( GetParent()->IsKindOf(RUNTIME_CLASS(CxImageViewCtrl)) )
	{
		CWnd* pParentWnd = GetParent()->GetParent();
		if ( pParentWnd && pParentWnd->IsTopParentActive() )
		{
			GetParent()->SetFocus();
		}
	}

	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_MBUTTONDOWN, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_MBUTTONDOWN, point ) )
			return;
	}

	CView::OnMButtonDown(nFlags, point);
}

void CxImageView::OnMButtonUp(UINT nFlags, CPoint point) 
{
	if (!m_bEnableMouseControl)
		return;

	if ( m_fnOnFireMouseEvent )
	{
		if ( (*m_fnOnFireMouseEvent)( WM_MBUTTONUP, point, m_nIndexData, m_lpUsrDataOnFireMouseEvent ) )
			return;
	}
	else
	{
		if ( OnFireMouseEvent( WM_MBUTTONUP, point ) )
			return;
	}

	
	CView::OnMButtonUp(nFlags, point);
}

void CxImageView::DrawElapsedTime( HDC hDC, RECT rc )
{
	CDC* pDC = CDC::FromHandle( hDC );
	m_dDrawElapsedTime;

	DWORD dwOldTextColor = pDC->SetTextColor( RGB(0xff, 0xff, 0xff) );
	int nOldBkMode = pDC->SetBkMode( TRANSPARENT );

	CRect rcBlock = rc;
	pDC->SetTextColor( RGB(0xff, 0xff, 0xff) );

	CString strText;
	strText.Format( _T("%.1fms"), m_dDrawElapsedTime );
	rcBlock.left--; rcBlock.right--;
	pDC->DrawText( strText, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	rcBlock.left+=2; rcBlock.right+=2;
	pDC->DrawText( strText, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	rcBlock.left--; rcBlock.right--;
	rcBlock.top--; rcBlock.bottom--;
	pDC->DrawText( strText, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	rcBlock.top+=2; rcBlock.bottom+=2;
	pDC->DrawText( strText, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	rcBlock.top--; rcBlock.bottom--;
	
	pDC->SetTextColor( RGB(50, 50, 50) );
	pDC->DrawText( strText, &rcBlock, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	
	pDC->SetTextColor( dwOldTextColor );
	pDC->SetBkMode( nOldBkMode );
}

void CxImageView::DrawScaleMark( HDC hDC, RECT rc, float fZoomRatio, LPCTSTR lpctszMeasures )
{
	float fScale = (float)((rc.right - rc.left) / fZoomRatio);
	if ( _isnan(fScale) || !_finite(fScale) ||
		fScale < 1e-37f || fScale > 1e37f )
	{
		return;
	}
	BOOL bMinusScale = FALSE;
	if ( fScale < 0.f )
	{
		fScale *= -1.f;
		bMinusScale = TRUE;
	}

	int nExpCount = 0;
	while ( fScale < 1.f || fScale >= 10.f )
	{
		if ( fScale >= 10.f )
		{
			fScale /= 10.f;
			nExpCount++;
		}
		if ( fScale < 1.f )
		{
			fScale *= 10.f;
			nExpCount--;
		}
	}

	int nSplit;
	if ( fScale < 1.5 )
		fScale = 1.0f, nSplit = 2;
	else if ( fScale < 2.0 )
		fScale = 1.5f, nSplit = 3;
	else if ( fScale < 3.0 )
		fScale = 2.0f, nSplit = 2;
	else if ( fScale < 5.0 )
		fScale = 3.0f, nSplit = 3;
	else if ( fScale < 8.0 )
		fScale = 5.0f, nSplit = 1;
	else
		fScale = 8.0f, nSplit = 2;

	int nWidthScaleMark = (int)(fScale * (float)pow(10., nExpCount) * fZoomRatio);

	TCHAR szUnitForm[24]; szUnitForm[0] = 0;
	//fScale;
	if ( nExpCount >= 3 )
	{
		if ( nExpCount >= 9 )
		{
			nExpCount -= 9;
			_stprintf(szUnitForm, _T("G"));
		}
		else if ( nExpCount >= 6 )
		{
			nExpCount -= 6;
			_stprintf(szUnitForm, _T("M"));
		}
		else
		{
			nExpCount -= 3;
			_stprintf(szUnitForm, _T("k"));
		}
	}
	else if ( nExpCount <= -3 )
	{
		if ( nExpCount <= -7 )
		{

			nExpCount += 9;
			_stprintf(szUnitForm, _T("n"));
		}
		else if ( nExpCount <= -4 )
		{
			nExpCount += 6;
			_stprintf(szUnitForm, _T("u"));
		}
		else
		{
			nExpCount += 3;
			_stprintf(szUnitForm, _T("m"));
		}
	}
	if ( lpctszMeasures == NULL )
	{
		_tcscat(szUnitForm, _T("m"));
	}
	else
	{
		_tcscat(szUnitForm, lpctszMeasures);
	}

	for ( int ni = 0; ni < nExpCount; ni++ )
	{
		fScale *= 10.0;
	}
	for ( int ni = 0; ni > nExpCount; ni-- )
	{
		fScale /= 10.0;
	}

	// draw scale mark
	rc.left++;
	rc.right = rc.left + nWidthScaleMark;

	DrawScaleMark_Simple0(hDC, rc, fScale, szUnitForm, nSplit);
}

BOOL CxImageView::DrawScaleMark_Simple0( HDC hDC, RECT rc, float fScale, LPCTSTR lpctszUnitForm, int nSplit )
{
	TCHAR szScale[72];
	_stprintf(szScale, _T("%.3f"), fScale);
	int ni;
	for ( ni = (int)_tcslen(szScale) - 1; ni >= 0; ni-- )
	{
		if ( szScale[ni] != '0' ) break;
		szScale[ni] = 0;
	}
	if ( ni >= 0 && szScale[ni] == '.' ) szScale[ni] = 0;
	_tcscat(szScale, lpctszUnitForm);
	int nScaleLen = (int)_tcslen(szScale);

	int nTextHeight = rc.top - rc.bottom;
	int nUnitY = 1;
	if ( nTextHeight < 0 ) nUnitY = -1;
	nTextHeight /= 2;
	int nScaleBarHeight = (rc.top - rc.bottom) - nTextHeight;
	int nSharpLine = nScaleBarHeight / 20; nSharpLine *= nUnitY;
	if ( nSharpLine == 0 ) nSharpLine = 1;
	int nBoldLine = nSharpLine * 2;
	int nScaleBarHeightCenter = nScaleBarHeight / 2;

	// Draw BackGround Text
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = nTextHeight;
	if ( lf.lfHeight > 0 ) lf.lfHeight *= -1;
	lf.lfWidth = nTextHeight * 7 / 12;
	if ( lf.lfWidth > 0 ) lf.lfWidth *= -1;
	lf.lfOrientation = 0;
	lf.lfEscapement = 0;
	lf.lfWeight = FW_BOLD;
	lf.lfItalic = lf.lfUnderline = lf.lfStrikeOut = 0;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(lf.lfFaceName, _T(""));
	HGDIOBJ hNewFont, hFondOld = NULL;
	if ( (hNewFont = ::CreateFontIndirect(&lf)) != NULL )
	{
		hFondOld = ::SelectObject(hDC, hNewFont);
	}
	int nBkModeOld = ::SetBkMode(hDC, TRANSPARENT);
	COLORREF clrTextOld = ::SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
	RECT rcText = {rc.left, rc.bottom + nScaleBarHeight, rc.right, rc.top};
	::OffsetRect(&rcText, 1, 0);
	::DrawText(hDC, szScale, nScaleLen, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	::OffsetRect(&rcText, -2, 0);
	::DrawText(hDC, szScale, nScaleLen, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	::OffsetRect(&rcText, 1, 1);
	::DrawText(hDC, szScale, nScaleLen, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	::OffsetRect(&rcText, 0, -2);
	::DrawText(hDC, szScale, nScaleLen, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);
	::OffsetRect(&rcText, 0, 1);

	// Draw BackGround
	int nBkColorOld = ::SetBkColor(hDC, RGB(0xff, 0xff, 0xff));
	RECT rcBar = {rc.left - 1, rc.bottom + nScaleBarHeight, rc.left + 1 + nSharpLine, rc.bottom};
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	rcBar.left = rc.right - 1 - nSharpLine; rcBar.right = rc.right + 1;
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	rcBar.bottom = rc.bottom + nScaleBarHeightCenter;
	for ( ni = 1; ni < nSplit; ni++ )
	{
		int nS = (rc.right - rc.left - 1) * ni / nSplit;
		rcBar.left = nS + rc.left - 1; rcBar.right = nS + rc.left + 1 + nSharpLine;
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	}
	rcBar.left = rc.left; rcBar.right = rc.right;
	rcBar.bottom = rc.bottom + nScaleBarHeightCenter - (nSharpLine * nUnitY) - nUnitY;
	rcBar.top = rcBar.bottom + (nBoldLine * nUnitY) + nUnitY + nUnitY;
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);

	// Draw BackGround Text
	::SetTextColor(hDC, RGB(50, 50, 50));
	::DrawText(hDC, szScale, nScaleLen, &rcText, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP);

	// Draw Line
	::SetBkColor(hDC, RGB(50, 50, 50));
	rcBar.left = rc.left; rcBar.top = rc.bottom + nScaleBarHeight;
	rcBar.right = rc.left + nSharpLine; rcBar.bottom = rc.bottom;
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	rcBar.left = rc.right - nSharpLine; rcBar.right = rc.right;
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	rcBar.bottom = rc.bottom + nScaleBarHeightCenter;
	for ( ni = 1; ni < nSplit; ni++ )
	{
		int nS = (rc.right - rc.left - 1) * ni / nSplit;
		rcBar.left = nS + rc.left; rcBar.right = nS + rc.left + nSharpLine;
		::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);
	}
	rcBar.left = rc.left; rcBar.right = rc.right;
	rcBar.bottom = rc.bottom + nScaleBarHeightCenter - (nSharpLine * nUnitY);
	rcBar.top = rcBar.bottom + (nBoldLine * nUnitY);
	::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rcBar, NULL, 0, NULL);

	::SetBkColor(hDC, nBkColorOld);
	::SetTextColor(hDC, clrTextOld);
	::SetBkMode(hDC, nBkModeOld);
	if ( hFondOld ) ::SelectObject(hDC, hFondOld);
	if ( hNewFont ) ::DeleteObject(hNewFont);

	return TRUE;
}

void CxImageView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if ( m_eScreenMode == ImageViewMode::ScreenModeZoomInOut )
		return;

	if (!m_bEnableMouseControl)
		return;

	CPoint ptHit = point;
	ScreenToClient( &ptHit );
	if ( m_pInnerUI->m_RectTracker.HitTest( ptHit ) != CRectTracker::hitNothing )
	{
		m_pInnerUI->m_ROIPopupMenu.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this, NULL );
		return;
	}

	m_pInnerUI->m_WheelPopupMenu.TrackPopupMenu( TPM_LEFTALIGN, point.x, point.y, this, NULL );
}

void CxImageView::OnContextMenuHandler( UINT uID )
{
	switch ( uID )
	{
	case ID_TRACKER_CLEAR:
		m_pInnerUI->m_RectTracker.m_rect = CRect(-100,-100,-100,-100);
		Invalidate();
		break;
	case ID_TRACKER_CONFIRM_REGION:
		{
			CRect rcTrack;
			
			CPoint ptTL = m_pInnerUI->m_RectTracker.m_rect.TopLeft();
			CPoint ptBR = m_pInnerUI->m_RectTracker.m_rect.BottomRight();
			ptTL = MousePosToImagePos( ptTL.x, ptTL.y );
			ptBR = MousePosToImagePos( ptBR.x, ptBR.y );
			rcTrack = CRect( ptTL, ptBR );
			if ( m_fnOnConfirmTracker )
			{
				(*m_fnOnConfirmTracker)( rcTrack, m_nIndexData, m_lpUsrDataOnConfirmTracker );
			}
			else
			{
				OnConfirmTracker( rcTrack );
			}
			Invalidate();
		}
		break;
	case ID_WHEEL_SET_TO_ZOOM:
		m_eMouseWheelMode = ImageViewMode::MouseWheelModeZoom;
		break;
	case ID_WHEEL_SET_TO_VSCROLL:
		m_eMouseWheelMode = ImageViewMode::MouseWheelModeVerticalScroll;
		break;
	case ID_WHEEL_SET_TO_HSCROLL:
		m_eMouseWheelMode = ImageViewMode::MouseWheelModeHorizontalScroll;
		break;
	case ID_WHEELBTN_SET_TO_SMART_MOVE:
		m_bEnableSmartMove = !m_bEnableSmartMove;
		break;
	case ID_LBUTTON_LIMIT_LOCK:
		m_bEnableLockMouse = !m_bEnableLockMouse;
		break;
	}
	WheelMenuUpdateUI();
}

void CxImageView::WheelMenuUpdateUI()
{
	switch ( m_eMouseWheelMode )
	{
	case ImageViewMode::MouseWheelModeZoom:
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_ZOOM, MF_BYCOMMAND|MF_CHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_VSCROLL, MF_BYCOMMAND|MF_UNCHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_HSCROLL, MF_BYCOMMAND|MF_UNCHECKED);
		break;
	case ImageViewMode::MouseWheelModeVerticalScroll:
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_ZOOM, MF_BYCOMMAND|MF_UNCHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_VSCROLL, MF_BYCOMMAND|MF_CHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_HSCROLL, MF_BYCOMMAND|MF_UNCHECKED);
		break;
	case ImageViewMode::MouseWheelModeHorizontalScroll:
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_ZOOM, MF_BYCOMMAND|MF_UNCHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_VSCROLL, MF_BYCOMMAND|MF_UNCHECKED);
		m_pInnerUI->m_WheelPopupMenu.CheckMenuItem(ID_WHEEL_SET_TO_HSCROLL, MF_BYCOMMAND|MF_CHECKED);
		break;
	}
}

void CxImageView::SetPalette( const BYTE* pPal )
{
	m_pDirectDIB->SetPalette( pPal );
}

int CxImageView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect rcClient;
	GetClientRect( &rcClient );
	
	// initialize renderer
	InitGraphics( m_pDirectDIB, rcClient.Width(), rcClient.Height(), 8 );
	//InitGraphics( &m_DirectDraw, rcClient.Width(), rcClient.Height(), 8 );

	m_pDrawDIB->SetDevice( m_pRenderer );

	WheelMenuUpdateUI();

//	m_pPaintThread->Start();
	
	return 0;
}

void CxImageView::OnTimer(UINT_PTR nIDEvent) 
{
	if ( nIDEvent == 1 )
	{
		if ( !m_bMouseOverCheck ) return;
		
		CRect rc;
		CPoint pt( GetMessagePos() );
		ScreenToClient( &pt );
		GetClientRect( rc );
				
		if ( !rc.PtInRect(pt) )
		{
			KillTimer(1);
			m_bMouseOverCheck = FALSE;
			if ( m_fnOnFireMouseEvent )
			{
				(*m_fnOnFireMouseEvent)( WM_MOUSELEAVE, CPoint(0,0), m_nIndexData, m_lpUsrDataOnFireMouseEvent );
			}
		}
	}	
	CView::OnTimer(nIDEvent);
}

void CxImageView::OnPaint()
{
	CPaintDC dc(this);

	OnDraw( &dc );
}
