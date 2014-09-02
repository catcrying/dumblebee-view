/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#if !defined(AFX_POINTCLIPPER_H__ABC50CA8_951F_4CA0_8000_5D9C2FCC1940__INCLUDED_)
#define AFX_POINTCLIPPER_H__ABC50CA8_951F_4CA0_8000_5D9C2FCC1940__INCLUDED_

#include <XGraphic/export.h>
#include <XGraphic/xDataTypes.h>
#include <wtypes.h>
#include <tchar.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class XGRAPHIC_API CxPointClipper  
{
private:
	LPDPOINT	m_lpDPoints;
	LPPOINT		m_lpPoints;
	int			m_nBufferedCount;
	LPDPOINT	m_lpTempPointBuffer;
	int			m_nTempBufferedCount;
	int			m_nCount;
	DRECT		m_rcBound;
	BOOL		m_bClosed;
public:
	CxPointClipper() :	m_nBufferedCount(0), m_nTempBufferedCount(0), m_nCount(0), m_bClosed(FALSE), 
						m_lpDPoints(NULL), m_lpPoints(NULL), m_lpTempPointBuffer(NULL) {}
	~CxPointClipper();

protected:
	void GetCrossedEdge(const double dPtX1, const double dPtY1, const double dPtX2, const double dPtY2, 
								   double& dCrossedPointY, const double dX );
	void ClipTop();
	void ClipBottom();
	void ClipRight();
	void ClipLeft();

	int GetClipPointCountTop();
	int GetClipPointCountBottom();
	int GetClipPointCountRight();
	int GetClipPointCountLeft();

	void AllocatePointBuffer( int nCount );
	void AllocateTempPointBuffer( int nCount );

	bool CheckOutOfBound() const;

	void TrimPoints();
public:

	BOOL ClipPoints( DRECT& rcBound, const LPDPOINT lpPoints, int nCount, BOOL bClose=FALSE );

	const int GetClipPointCount() { return m_nCount; }
	const LPDPOINT GetClipDPoints() { return m_lpDPoints; }
	const LPPOINT GetClipPoints();
};

#endif // !defined(AFX_POINTCLIPPER_H__ABC50CA8_951F_4CA0_8000_5D9C2FCC1940__INCLUDED_)
