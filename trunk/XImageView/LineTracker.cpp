// LineTracker.cpp: implementation of the CLineTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LineTracker.h"

#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLineTracker::CLineTracker(int nHandleSize, COLORREF rgbHandleColor)
{
	// Init current line and points
	m_pCurrentLine = NULL;
	m_points.SetRectEmpty();

	// Set handle attributes
	m_nHandleSize = nHandleSize;
	m_rgbHandle = rgbHandleColor;


	CWinApp* pApp = AfxGetApp();

	ASSERT(pApp != NULL);

	// Load the cursors we will use
	m_hcHandles[hitStart]	= pApp->LoadStandardCursor(IDC_SIZENWSE);
	m_hcHandles[hitEnd]		= pApp->LoadStandardCursor(IDC_SIZENESW);
	m_hcHandles[hitMiddle]	= pApp->LoadStandardCursor(IDC_SIZEALL);
}

CLineTracker::~CLineTracker()
{

}

BOOL CLineTracker::SetCursor(CWnd *pWnd, UINT nHitTest)
{

	if (nHitTest != HTCLIENT)
		return FALSE;

	// Get mouse position
	CPoint ptMouse;
	::GetCursorPos(&ptMouse);
	
	// Convert to client coords of pWnd
	pWnd->ScreenToClient(&ptMouse);

	// Hit test the handles
	int nHandle;

	nHandle = HitTest(ptMouse);

	if (nHandle != hitNothing) {

		::SetCursor(m_hcHandles[nHandle]);

		return TRUE;

	}

	return FALSE;
}

void CLineTracker::Draw(CDC *pDC, CPoint& ptScrollOffset)
{

	// If there are no lines just return
	if (m_LineObjects.GetSize() == 0)
		return;

	CBrush	brush;
	CBrush* pOldBrush;

	brush.CreateSolidBrush(m_rgbHandle);

	pOldBrush = pDC->SelectObject(&brush);

	CRect handleRect;
	CRect rect;
	CLineItem* pLine;

	for (int i = 0; i < m_LineObjects.GetSize(); i++) {

		pLine = m_LineObjects.GetAt(i);

		rect = pLine->m_rcPoints;

		rect += ptScrollOffset;
		
		// Draw the resize handles
		handleRect.left = ((rect.left) - m_nHandleSize / 2);
		handleRect.right = handleRect.left + m_nHandleSize;

		handleRect.top = ((rect.top) - m_nHandleSize / 2);
		handleRect.bottom = handleRect.top + m_nHandleSize;

		pDC->Rectangle(&handleRect);

		handleRect.left = ((rect.right) - m_nHandleSize / 2);
		handleRect.right = handleRect.left + m_nHandleSize;

		handleRect.top = ((rect.bottom) - m_nHandleSize / 2);
		handleRect.bottom = handleRect.top + m_nHandleSize;
		
		pDC->Rectangle(&handleRect);

	}

	pDC->SelectObject(pOldBrush);

	brush.DeleteObject();

}

BOOL CLineTracker::TrackRubberLine(CWnd *pWnd, CPoint point)
{

	// Make sure no capture is set
	if (CWnd::GetCapture() != NULL)
		return FALSE;

	// Flag for erasing the last line
	BOOL bErase = FALSE;
	
	CRect points;

	// Set start point
	points.left = point.x;
	points.top	= point.y;

	// Get DC to draw
	CDC* pDC;

	pDC = pWnd->GetDC();

	// Capture the mouse
	pWnd->SetCapture();

	CRect rcLastLine;

	MSG msg;

	for (;;) {

		::GetMessage(&msg, NULL, 0, 0);

		switch (msg.message) {

		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:

			points.right	= LOWORD(msg.lParam);
			points.bottom	= HIWORD(msg.lParam);

			if (msg.message == WM_LBUTTONUP)
				goto ExitLoop;

			if (bErase)
				DrawTrackerLine(&rcLastLine, pDC);

			DrawTrackerLine(&points, pDC);

			rcLastLine = points;

			bErase = TRUE;

			break;

		case WM_RBUTTONDOWN:

			ReleaseCapture();

			return FALSE;

			break;

		case WM_KEYDOWN:

			if (msg.wParam == VK_ESCAPE) {

				ReleaseCapture();

				return FALSE;

			}

		}

	}

ExitLoop:

	ReleaseCapture();

	// Update points
	m_points = points;

	return TRUE;
}

BOOL CLineTracker::Track(CWnd *pWnd, CPoint point, CWnd *pWndClipTo)
{

	int nHandle;

	nHandle = HitTest(point);

	if (nHandle == hitNothing)
		return FALSE;

	return TrackHandle(nHandle, pWnd, point, pWndClipTo);
}

void CLineTracker::Add(CLineItem *pLine)
{

	m_LineObjects.Add(pLine);

}

int CLineTracker::HitTest(CPoint point)
{

	// Hit test all handles
	CRect		handleRect;
	CLineItem*	pLine = NULL;

	// Return value
	int nRet = hitNothing;

	for (int i = 0; i < m_LineObjects.GetSize(); i++) {

		pLine = m_LineObjects.GetAt(i);

		m_points = pLine->m_rcPoints;

		// Did we hit the start handle?
		GetHandleRect(handleStart, &handleRect);

		if (handleRect.PtInRect(point)) {

			nRet = hitStart;

			break;

		}

		// Did we hit the end handle?
		GetHandleRect(handleEnd, &handleRect);

		if (handleRect.PtInRect(point)) {

			nRet = hitEnd;

			break;

		}

		// Did we hit the line?
		if (HitTestLine(point)) {

			nRet = hitMiddle;

			break;

		}
	}

	if (nRet != hitNothing) {

		m_pCurrentLine	= pLine;

	} else {

		m_pCurrentLine = NULL;
		m_points.SetRectEmpty();

	}

	return nRet;
}

void CLineTracker::GetHandleRect(int nHandle, CRect *pHandleRect)
{

	CRect handleRect;
	CRect rect = m_points;

	switch (nHandle) {

	case handleStart:

		// Calc the start handle rect
		handleRect.left = ((rect.left) - m_nHandleSize / 2);
		handleRect.right = handleRect.left + m_nHandleSize;

		handleRect.top = ((rect.top) - m_nHandleSize / 2);
		handleRect.bottom = handleRect.top + m_nHandleSize;

		*pHandleRect = handleRect;

	break;

	case handleEnd:

		// Calc the end handle rect
		handleRect.left = ((rect.right) - m_nHandleSize / 2);
		handleRect.right = handleRect.left + m_nHandleSize;

		handleRect.top = ((rect.bottom) - m_nHandleSize / 2);
		handleRect.bottom = handleRect.top + m_nHandleSize;

		*pHandleRect = handleRect;
		
		break;

	default:

		pHandleRect->SetRectEmpty();

		break;

	}

}

BOOL CLineTracker::HitTestLine(CPoint point)
{

	int dxap = point.x - m_points.left;			// Vector AP
	int dyap = point.y - m_points.top;
	int dxab = m_points.right - m_points.left;	// Vector AB
	int dyab = m_points.bottom - m_points.top;

	double ab2 = dxab*dxab + dyab*dyab; // Magnitude of AB

	double t; // This will hold the parameter for the Point of projection of P on AB

	if (ab2 <= 2) {

		t=0;   // A and B coincide
	
	} else {
		
		t = (dxap*dxab + dyap*dyab) / ab2;

	}

 // Above equation maps to (AP dot normalized(AB)) / magnitude(AP dot normalized(AB))

	if (t<0) {
		
		t = 0;   // Projection is beyond A so nearest point is A
	
	} else { 
		
		if (t>1) t = 1; // Projection is beyond B so nearest point is B

	}

	double xf = m_points.left + t * dxab; // Projection point on Seg AB
	double yf = m_points.top  + t * dyab; //

	double dxfp = point.x - xf;
	double dyfp = point.y - yf;

	int nDist;

	nDist = (int)sqrt(dxfp*dxfp + dyfp*dyfp);

	if ( nDist < 4) {

		return TRUE;

	}

	 return FALSE;
}

BOOL CLineTracker::TrackHandle(int nHandle, CWnd *pWnd, CPoint point, CWnd *pWndClipTo)
{

	// Flag for erasing lines
	BOOL bErase = FALSE;

	// Mouse already captured?
	if (::GetCapture() != NULL)
		return FALSE;

	// Set pointers to the end that we are dragging
	long *px = NULL;
	long *py = NULL;

	switch (nHandle) {

	case hitStart:

		px = &m_points.left;
		py = &m_points.top;
	
		break;

	case hitEnd:

		px = &m_points.right;
		py = &m_points.bottom;

		break;

	}
	
	// Save the original point (in case we cancel operation)
	CRect	rcSaved;

	// Save points in case the operation is cancelled
	rcSaved = m_points;

	// Get the DC for drawing
	CDC* pDC;

	if (pWndClipTo) {

		pDC = pWndClipTo->GetDCEx(NULL, DCX_CACHE);

	} else {

		pDC = pWnd->GetDC();

	}

	ASSERT_VALID(pDC);

	// Set the capture
	pWnd->SetCapture();

	// Save the mouse point for reference
	CPoint ptFirstClick = point;

	// Process messages until capture lost or user cancels
	for (;;) {

		MSG msg;

		// Get a message
		::GetMessage(&msg, NULL, 0, 0);

		// Did we lose the capture?
		if (CWnd::GetCapture() != pWnd)
			break;

		// See which message we got

		switch (msg.message) {

		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:

			// Line(s) to erase?
			if (bErase) {

				DrawTrackerLine(&m_points, pDC);

			}

			// Set new point x, y coords
			if (px != NULL && py != NULL) {

				*px = LOWORD(msg.lParam);
				*py = HIWORD(msg.lParam);

			}

			// Handle moving of the line
			if (nHandle == hitMiddle) {
				
				// Find the difference between this point and the last one
				int dx = LOWORD(msg.lParam) - ptFirstClick.x;
				int dy = HIWORD(msg.lParam) - ptFirstClick.y;

				// Adujust the line position	
				m_points.left	+= dx;
				m_points.top	+= dy;
				m_points.right	+= dx;
				m_points.bottom	+= dy;

				// Update the last clicked position
				ptFirstClick.x = LOWORD(msg.lParam);
				ptFirstClick.y = HIWORD(msg.lParam);

			}

			// Draw tracker line
			DrawTrackerLine(&m_points, pDC);
			
			// Save erase flag
			bErase = TRUE;
			
			if (msg.message == WM_LBUTTONUP) {

				goto ExitLoop;
			}

			break;

		case WM_RBUTTONDOWN:

			ReleaseCapture();

			m_pCurrentLine->m_rcPoints = rcSaved;

			return FALSE;

			break;

		case WM_KEYDOWN:

			if (msg.wParam == VK_ESCAPE) {

				ReleaseCapture();
				
				m_pCurrentLine->m_rcPoints = rcSaved;

				return FALSE;

			}

		default:

			DispatchMessage(&msg);

			break;
		}

	}

ExitLoop:

	ReleaseCapture();

	m_pCurrentLine->m_rcPoints = m_points;

	return TRUE;
}

void CLineTracker::DrawTrackerLine(CRect *pRect, CDC *pDC)
{

	int nPrevMode;

	nPrevMode = pDC->SetROP2(R2_NOT);

	pDC->MoveTo(pRect->left, pRect->top);
	pDC->LineTo(pRect->right, pRect->bottom);

	pDC->SetROP2(nPrevMode);
}


// Removes all line objects from the tracker selection
void CLineTracker::RemoveAll(void)
{

	m_LineObjects.RemoveAll();

}

// Returns the current line object selected
CLineTracker::CLineItem* CLineTracker::GetSelection(void)
{
	return m_pCurrentLine;
}

BOOL CLineTracker::OnLButtonDown(CWnd* pWnd, unsigned int nFlags, CPoint point, CLineList* pLineList)
{
	// Our function return value
	BOOL bRet = FALSE;
	
	// Remove selection
	RemoveAll();

	CLineItem* pLine;

	pLine = pLineList->GetFirst();

	while (pLine) {

		m_points = pLine->m_rcPoints;

		// Did we hit a line?
		if (HitTestLine(point)) {

			// Is this line already selected?
			if (IsSelected(pLine)) {

				Remove(pLine);
			
			} else {

				Add(pLine);

			}

			bRet = TRUE;
		}

	pLine = pLine->m_pNext;

	}
	
	// Are we tracking a handle?
	if (Track(pWnd, point)) {

		bRet = TRUE;

	};	

	return bRet;

}

BOOL CLineTracker::IsSelected(CLineItem* pLine)
{

	for (int i = 0; i < m_LineObjects.GetSize(); i++) {

		if (m_LineObjects.GetAt(i) == pLine)
			return TRUE;

	}

	return FALSE;
}

void CLineTracker::Remove(CLineItem* pLine)
{
	for (int i = 0; i < m_LineObjects.GetSize(); i++) {

		if (m_LineObjects.GetAt(i) == pLine) {

			m_LineObjects.RemoveAt(i);

			return;

		}

	}
}

CLineTracker::CLineItem::CLineItem()
{

	m_rcPoints.SetRectEmpty();

	m_crColor = RGB(0,0,0);

	m_pNext = NULL;

}

CLineTracker::CLineItem::~CLineItem()
{

}

CLineTracker::CLineList::CLineList()
{

	m_pFirstItem = NULL;
}

CLineTracker::CLineList::~CLineList()
{

	// Delete all items
	CLineItem *pLine, *pLineTmp;

	pLine = m_pFirstItem;

	while (pLine) {

		pLineTmp = pLine->m_pNext;

		delete pLine;

		pLine = pLineTmp;

	}
		
}

void CLineTracker::CLineList::Add(CLineItem *pLine)
{

	// Is this the first item?
	if (m_pFirstItem == NULL) {

		m_pFirstItem = pLine;

		return;

	}

	// Walk the list until we find the end
	CLineItem* pTemp = m_pFirstItem;

	while (pTemp->m_pNext != NULL) {

		pTemp = pTemp->m_pNext;

	}

	// Ok, we found the end of the list
	pTemp->m_pNext = pLine;

}

CLineTracker::CLineItem* CLineTracker::CLineList::GetFirst()
{

	return m_pFirstItem;
}

void CLineTracker::CLineList::RemoveAll(void)
{
	CLineItem* pLine;
	CLineItem* pTemp;

	pLine = m_pFirstItem;

	while (pLine) {

		pTemp = pLine->m_pNext;

		delete pLine;

		pLine = pTemp;

	}

	m_pFirstItem = NULL;

}

