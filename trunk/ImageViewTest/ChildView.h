
// ChildView.h : interface of the CChildView class
//


#pragma once

#include <XImageView/xImageViewCtrl.h>
#include <XImage/xImageObject.h>
#include <XGraphic/xGraphicObject.h>

// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CxImageViewManager	m_wndImageViewManager;
	CxImageViewSyncManager	m_wndImageViewSyncManager;
	CxImageViewCtrl		m_wndImageView[2];
	CxImageObject		m_ImageObject[2];

	CxGraphicObject		m_GraphicObject[2];
// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
};

