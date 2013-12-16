
// ImageViewTest.h : main header file for the ImageViewTest application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CImageViewTestApp:
// See ImageViewTest.cpp for the implementation of this class
//

class CImageViewTestApp : public CWinApp
{
public:
	CImageViewTestApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CImageViewTestApp theApp;
