#ifndef __IMAGEVIEW_CTRL_H__
#define __IMAGEVIEW_CTRL_H__

#include <XImageView/export.h>
#include <XImageView/xImageViewConst.h>

#include <gdiplus.h>

#define MBT_ZOOM_IN			0x00000001
#define MBT_ZOOM_OUT		0x00000002
#define MBT_ZOOM_FIT		0x00000004
#define MBT_ZOOM_NOT		0x00000008

#define MBT_ZOOM_INOUT		0x00000010
#define MBT_MEASURE			0x00000020

#define MBT_LOAD			0x00000100
#define MBT_SAVE			0x00000200

#define MBT_MAXIMIZE		0x00004000

#define	MBT_LIMIT			0x00010000

#define MBT_BUTTON_ALL		(MBT_ZOOM_IN|MBT_ZOOM_OUT|MBT_ZOOM_FIT|MBT_ZOOM_NOT|\
							 MBT_ZOOM_INOUT|MBT_MEASURE|\
							 MBT_LOAD|MBT_SAVE|\
							 MBT_MAXIMIZE)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CxImageViewCtrl.h : header file
//

//////////////////////////////////////////////////////////////////////////
// CxImageViewManager
class CxImageViewCtrl;
class CxImageViewArray;
class XIMAGE_VIEW_API CxImageViewManager
{
protected:
	CxImageViewArray*			m_pViewArray;

public:
	CxImageViewManager();
	~CxImageViewManager();
	void AddView( CxImageViewCtrl* pView );
	void MaximizeWindow( CxImageViewCtrl* pView );
	void NormalWindow( CxImageViewCtrl* pView );

	void Reset();
};

class CxImageViewCtrl;
class IxDeviceContext;
class XIMAGE_VIEW_API CxImageViewSyncManager
{
protected:
	CxImageViewArray*			m_pViewArray;
	BOOL	m_bFixZoom;
	BOOL	m_bMouseTrackPoint;
public:
	CxImageViewSyncManager();
	~CxImageViewSyncManager();
	void AddView( CxImageViewCtrl* pView );

	void Reset();

	void SyncDevContext( CxImageViewCtrl* pView, IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage );

	BOOL IsTrackMousePoint();
	void SetTrackMousePoint( BOOL bTrack );

	BOOL IsFixZoom();
	void SetFixZoom( BOOL bFix );
};

/////////////////////////////////////////////////////////////////////////////
// CxImageViewCtrl
class CxImageScrollView;
class CxGraphicObject;
class CxImageObject;

class XIMAGE_VIEW_API CxImageViewCtrl : public CWnd
{
public:
	CxImageViewCtrl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CxImageViewCtrl();
	DECLARE_DYNCREATE(CxImageViewCtrl)
	BOOL Create( CxImageViewManager* pViewManager, CWnd* pParentWnd = NULL );

	//////////////////////////////////////////////////////////////////////////
	// Behavior
	void SetMiniButtonType( DWORD dwMBType, BOOL bMaximize );
	void SetMiniButtonTypeAll( DWORD dwMBType );
	void SetTitleText( LPCTSTR lpszTitle );

	void SyncDevContext( IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage );

	void SetEnableMoveWindow( BOOL bEnable );
	void SetMaximizedWindowPos( RECT rcWindow );

	BOOL SetAnimateWindow( BOOL bEnable );

	void GetTitleRect( LPRECT lpRect );

	void SetTitleColor( DWORD dwBodyColor, DWORD dwInactiveTextColor, DWORD dwActiveTextColor );
	void SetButtonColor( DWORD dwHoverColor, DWORD dwPressColor,
						 DWORD dwBorderHoverColor, DWORD dwBorderPressColor );
	void SetStatusColor( DWORD dwBodyColor, DWORD dwTextColor );
	void SetBodyColor( DWORD dwBodyColor );		// gray only
	void SetBorderColor( DWORD dwOutlineBorderColor=(DWORD)-1, DWORD dwInnerTopBorderColor=(DWORD)-1, DWORD dwInnerBottomBorderColor=(DWORD)-1 );
	enum MiniButtonColorType { MiniButtonColorTypeBlack, MiniButtonColorTypeWhite, MiniButtonColorTypeColor };
	void SetMiniButtonColorType( MiniButtonColorType type );

	//////////////////////////////////////////////////////////////////////////
	// Set callback functions
	void SetOnDrawExt( FnOnDrawExt _fnOnDrawExt, LPVOID _lpUsrData );
	void SetOnMeasure( FnOnMeasure _fnOnMeasure, LPVOID _lpUsrData );
	void SetOnEvent( FnOnEvent _fnOnEvent, LPVOID _lpUsrData );
	void SetOnConfirmTracker( FnOnConfirmTracker _fnOnConfirmTracker, LPVOID _lpUsrData );
	void SetOnFireMouseEvent( FnOnFireMouseEvent _fnOnFireMouseEvent, LPVOID _lpUsrData );

	// Helper
	void SetRegisterCallBack( UINT nIndexData, REGISTER_CALLBACK* pRegisterCB );

	//////////////////////////////////////////////////////////////////////////
	// Pixel resolution
	void SetRealSizePerPixel( float fRealPixelSize );
	float GetRealSizePerPixel() const;
	void SetRealSizePerPixelW( float fRealPixelSizeW ) const;
	float GetRealSizePerPixelW() const;
	void SetRealSizePerPixelH( float fRealPixelSizeH );
	float GetRealSizePerPixelH() const;

	//////////////////////////////////////////////////////////////////////////
	// Associate CxImageObject
	// assign image object -> CxImageViewCtrl's image object instance
	void SetImageBuffer( int nWidth, int nHeight, LPVOID lpBuf );
	// attach image object -> view's image object pointer
	void SetImageObject( CxImageObject* pImageObject, BOOL bZoomFit=TRUE );
	// get current viewing image object pointer
	CxImageObject* GetVisibleImageObject();
	// get image object -> CxImageViewCtrl's image object
	CxImageObject* GetImageObject();

	BOOL SaveImage( LPCTSTR lpszPathName );
	BOOL LoadImage( LPCTSTR lpszPathName );

	//////////////////////////////////////////////////////////////////////////
	// Screen control
	void ZoomFit( BOOL bCalcScrollBar = TRUE );
	void ZoomTo( CPoint ptImageCenter, float fZoom, BOOL bSyncControl=FALSE );

	void ImageUpdate();
	void RedrawTitle();
	void RedrawStatus();

	void AttachGraphicObject( CxGraphicObject* pGO );	// assign external graphic object
	CxGraphicObject* DetachGraphicObject();
	BOOL IsGraphicObjectAttached();
	
	CxGraphicObject& GetGraphicObject();	// return internal graphic object
	IxDeviceContext* GetIDeviceContext();

	CxImageViewManager* GetImageViewManager();
	void SetImageViewManager( CxImageViewManager* pImageViewManager );

	CxImageViewSyncManager* GetSyncManager();
	void SetSyncManager( CxImageViewSyncManager* pSyncManager );

	//////////////////////////////////////////////////////////////////////////
	// Window control
	void ShowMaximizedWindow();
	void ShowNormalWindow();

	void ShowTitleWindow( BOOL bShow );
	void ShowStatusWindow( BOOL bShow );
	void ShowScaleBar( BOOL bShow );
	void ShowScrollBar( BOOL bShow );
	void ShowDigitize( BOOL bShow );
	void ShowDrawElapsedTime( BOOL bShow );
	void UseAutoFocus( BOOL bUse );
	void EnableMouseControl( BOOL bEnable );

	void EnableSyncDevContext( BOOL bSyncDevContext );

	BOOL IsMaximized();

	//////////////////////////////////////////////////////////////////////////
	// Tracker
	void SetTrackerPosition( CRect& rcTrack );
	void SetTrackerMode( BOOL bSet, BOOL bUseFixedTracker = FALSE );
	BOOL IsTrackerMode();

	void SetMouseWheelMode( ImageViewMode::MouseWheelMode eMouseWheelMode );
	ImageViewMode::MouseWheelMode GetMouseWheelMode() const;

	void SetPalette( const BYTE* pPal );

	void MoveWindow( int x, int y, int cx, int cy, BOOL bRepaint=TRUE, BOOL bZoomFit=TRUE );
	
protected:

	friend class CxImageViewManager;
	friend class CxImageViewSyncManager;
	friend class CxImageScrollView;
	friend class CxImageView;

	ImageViewMode::ScreenMode m_eOldScreenModeForTracker;

	BOOL	m_bIsModal;
	BOOL	m_bInActive;

	BOOL	m_bShowTitle;
	BOOL	m_bShowStatus;

	BOOL	m_bShowMiniButtons;

	CBrush		m_BodyBrush;
	COLORREF	m_dwBodyColor;
	COLORREF	m_dwBorderColors[3];		// 0: Outline, 1: InnerlineBottom, 2: InnerlineTop
	COLORREF	m_dwTitleBodyColor;
	COLORREF	m_dwActiveTitleColor;
	COLORREF	m_dwInactiveTitleColor;

	MiniButtonColorType m_eMiniButtonColorType;

	COLORREF	m_dwStatusBodyColor;
	COLORREF	m_dwStatusTextColor;

	COLORREF	m_dwButtonHoverColor;
	COLORREF	m_dwButtonPressColor;
	COLORREF	m_dwButtonBorderHoverColor;
	COLORREF	m_dwButtonBorderPressColor;

	int			m_nTitleRightLimit;
	int			m_nTitleBarHeight;

	CRect		m_rcMiniButton;

	CFont		m_TitleFont;

	DWORD		m_dwMiniButtonType[2];			// 0: NORMAL, 1:MAXIMIZE

#ifdef USE_NEW_VIEW
	CxImageView*	m_pImageView;
#else
	CxImageScrollView*	m_pImageView;
#endif
	
	void ArrangeInnerView( int cx, int cy );

	void HideAllMiniButtons();
	void ShowAllMiniButtons();
	
	void MaximizeWindow();
	void NormalWindow();

	void OnSyncDevContext( IxDeviceContext* pIDC, CPoint& ptImage, BOOL bUpdateImage );

	void OnStatusInfo( LONG lX, LONG lY, COLORREF dwColor, unsigned int nLevel, int nDepth, int nChannel );
	void OnStatusText( LPCTSTR lpszText );

	enum ButtonIconIndex
	{
		IndexNone = -1,
		IndexCamera = 0,
		IndexZoomIn = 1,
		IndexZoomOut = 2,
		IndexZoomInOut = 3,
		IndexZoomFit = 4,
		IndexZoomNot = 5,
		IndexLoad = 6,
		IndexSave = 7,
		IndexMeasure = 8,
		IndexShowIndexView = 9,
		IndexShowNormal = 10,
		IndexShowMaximize = 11,
		IndexMore = 12
	};

	Gdiplus::Rect m_rectMiniBtnBody[13];

	virtual void OnDraw( CDC& dc );

	void DrawTitle( Gdiplus::Graphics& g );
	void DrawStatus( Gdiplus::Graphics& g );
	void DrawBorder( Gdiplus::Graphics& g );
	void DrawMiniButton( Gdiplus::Graphics& g, const Gdiplus::RectF& rectBtn, ButtonIconIndex eIndex );

	CPoint	m_ptLastMouse;

	CString	m_strTitle;

	CxImageObject*	m_pImageObject;

	int				m_nZoomRatio;

	BOOL			m_bEnableMoveWindow;
	CRect			m_rcMaxWinPos;
	CRect			m_rcNormalWinPos;
	BOOL			m_bIsMaximized;

	BOOL			m_bAnimateWindow;

	BOOL			m_bSyncDevContext;

	CxImageViewManager*		m_pViewManager;
	CxImageViewSyncManager*	m_pSyncManager;

	CBitmap* m_pBitmap;
	CDC       m_MemDC;
	
	virtual BOOL InitControl();

	BOOL				m_bCreateInnerControl;
	void CreateInnerCtrls();

	FnOnEvent			m_fnOnEvent;
	LPVOID				m_lpUsrDataOnEvent;

	FnOnDrawExt			m_fnOnDrawExt;
	LPVOID				m_lpUsrDataOnDrawExt;

	FnOnMeasure			m_fnOnMeasure;
	LPVOID				m_lpUsrDataOnMeasure;

	FnOnConfirmTracker	m_fnOnConfirmTracker;
	LPVOID				m_lpUsrDataOnConfirmTracker;

	FnOnFireMouseEvent	m_fnOnFireMouseEvent;
	LPVOID				m_lpUsrDataOnFirMouseEvent;

	UINT				m_nIndexData;
	REGISTER_CALLBACK*	m_pRegisterCB;

	static void APIENTRY _OnDrawExt( IxDeviceContext* pIDC, CDC* pDC, UINT nIndexData, LPVOID lpUsrData );

	void LinkageIndexView( IxDeviceContext* pIDC );

	int m_nIndexViewWidth;
	int m_nIndexViewHeight;

	ButtonIconIndex GetIndexFromBtnMask( DWORD dwType );
	CString GetTextFromIndex(ButtonIconIndex index);
	
protected:

	CRect				m_rcStatus;
	COLORREF			m_dwPixelColor;
	unsigned int		m_nPixelLevel;
	CString				m_strStatus;
	CPoint				m_ptPixel;
	int					m_nPixelDepth;
	int					m_nChannel;

	CRect				m_rcTitle;
	Gdiplus::Image*		m_pMiniBtnIconWhite;
	Gdiplus::Image*		m_pMiniBtnIconBlack;
	Gdiplus::Image*		m_pMiniBtnIconColor;
	Gdiplus::Image*		m_pMiniBtnIcon;
	int					m_nMiniBtnIconSize;
	int					m_nMiniBtnIconCount;

	ButtonIconIndex		m_eZoomCheckedIndex;
	ButtonIconIndex		m_eButtonHoverIndex;
	ButtonIconIndex		m_eButtonPressIndex;

	ButtonIconIndex		m_eButtonFirst;
	ButtonIconIndex		m_eButtonLast;

	BOOL				m_bIsButtonDown;

	Gdiplus::FontFamily* m_pFontFamily;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxImageViewCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	void OnBtnLoad();
	void OnBtnSave();
	void OnBtnZoomFit();
	void OnBtnZoomIn();
	void OnBtnZoomOut();
	void OnBtnMaximize();
	void OnBtnZoomNot();
	void OnBtnZoomInout();
	void OnBtnMeasure();

	// Generated message map functions
	//{{AFX_MSG(CxImageViewCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnPaint();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseLeave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__IMAGEVIEW_CTRL_H__)
