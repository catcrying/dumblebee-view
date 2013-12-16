// ****************************************************
// LineTracker.h: interface for the CLineTracker class.
// ****************************************************
//
// This class was written by Dana Holt (xenosonline@hotmail.com)
// from Xenos Software (www.xenossoftware.com).
//
// You may use this class in any way you see fit.
// 
// This test class was built while working on a vector image editor.
// I hope it will prove as a useful start for someone that wants
// to implement this type of functionality.
//
// This version only supports one arrow selection at a time, but there
// are a few things here that I started working on to track multiple
// arrows.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINETRACKER_H__A3FF7A3E_0996_4E3F_8924_D67364CE3068__INCLUDED_)
#define AFX_LINETRACKER_H__A3FF7A3E_0996_4E3F_8924_D67364CE3068__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>

class CLineTracker  
{
public:
	class CLineItem  
	{
	public:
		CLineItem*	m_pNext;
		COLORREF	m_crColor;
		CRect		m_rcPoints;
		CLineItem();
		virtual ~CLineItem();
	
	};
	
	class CLineList  
	{
	public:
		CLineItem* GetFirst();
		void Add(CLineItem* pLine);
		CLineList();
		virtual ~CLineList();
	
	protected:
	
		CLineItem*	m_pFirstItem;
	
	public:
		void RemoveAll(void);
	};	
public:
	void GetHandleRect(int nHandle, CRect* pHandleRect);
	void Add(CLineItem* pLine);

	enum TrackerHit {

		hitNothing = -1,
		hitStart,
		hitEnd,
		hitMiddle

	};

	enum TrackerHandle {

		handleStart,
		handleEnd

	};


	CLineTracker(int nHandleSize = 7, COLORREF rgbHandleColor = RGB(0,255,0));
	virtual ~CLineTracker();
	// Tracks line
	BOOL Track(CWnd* pWnd, CPoint point, CWnd* pWndClipTo = NULL);
	// Draws a rubber line
	BOOL TrackRubberLine(CWnd* pWnd, CPoint point);
	// Draws the tracker line
	void Draw(CDC* pDC, CPoint& ptScrollOffset );
	// Handles mouse cursor changes
	BOOL SetCursor(CWnd* pWnd, UINT nHitTest);
	// Stores points of the current line
	CRect m_points;

protected:
	// The line we are currently working with
	CLineItem* m_pCurrentLine;
	// Draws a rubber line
	void DrawTrackerLine(CRect* pRect, CDC* pDC);
	// Track handle / move line
	BOOL TrackHandle(int nHandle, CWnd* pWnd, CPoint point, CWnd* pWndClipTo);
	// Mouse cursors
	HCURSOR m_hcHandles[3];
	// Size of the handles
	int m_nHandleSize;
	// Fill color of the handles
	COLORREF m_rgbHandle;

	CArray<CLineItem*, CLineItem*>	m_LineObjects;
private:

public:
	// Removes all line objects from the tracker selection
	void RemoveAll(void);
	// Returns the current line object selected
	CLineItem* GetSelection(void);
	// Returns true if the point is on the line
	BOOL HitTestLine(CPoint point);
	// Hit tests handles
	int HitTest(CPoint point);
	// Encapsulates logic for left mouse click
	BOOL OnLButtonDown(CWnd* pWnd, unsigned int nFlags, CPoint point, CLineList* pLineList);
	// Checks to see if line is already selected
	BOOL IsSelected(CLineItem* pLine);
	// Removes a line from the selected list
	void Remove(CLineItem* pLine);
};

#endif // !defined(AFX_LINETRACKER_H__A3FF7A3E_0996_4E3F_8924_D67364CE3068__INCLUDED_)
