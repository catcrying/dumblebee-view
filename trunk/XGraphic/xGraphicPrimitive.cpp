/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#include "stdafx.h"
#include <XGraphic/xGraphicPrimitive.h>

// region code
#define REGION_CODE_XLEFT   0x0001
#define REGION_CODE_XRIGHT  0x0002
#define REGION_CODE_XINSIDE 0x0004
#define REGION_CODE_XMASK   0x0007
#define REGION_CODE_YBOTTOM 0x0010
#define REGION_CODE_YTOP    0x0020
#define REGION_CODE_YINSIDE 0x0040
#define REGION_CODE_YMASK   0x0070
#define REGION_CODE_ZLEFT   0x0100
#define REGION_CODE_ZNEAR   0x0200
#define REGION_CODE_ZFAR    0x0400
#define REGION_CODE_ZMASK   0x0700

#ifndef M_PI
#define	M_PI		(3.1415926535897932384626433832795)
#endif

#ifndef M_PI2
#define M_PI2		(2. * M_PI )
#endif

inline double GetAngle( double x1, double y1, double x2, double y2 )
{	return -atan2( (double)( y2 - y1 ), (double) ( x2 - x1 ) ) * 360. / M_PI2;
}

XGRAPHIC_API void WINAPI InflatePolygon( const POINT& ptCenter, const UINT unptNum, 
									  POINT* pPolyPt, const int x, const int y )
{
	for ( UINT i = 0 ; i < unptNum ; i++ )
	{
		if ( pPolyPt[ i ].x < ptCenter.x ) pPolyPt[ i ].x -= (short)x;
		else pPolyPt[ i ].x += (short)x;

		if ( pPolyPt[ i ].y < ptCenter.y ) pPolyPt[ i ].y -= (short)y;
		else pPolyPt[ i ].y += (short)y;

	}
}

XGRAPHIC_API void WINAPI InflatePolygon( const POINT& ptCenter, const UINT unptNum, 
									  POINT* pPolyPt, const double dMagnitude )
{
	for ( UINT i = 0; i < unptNum; i++ )
	{
		double dx = pPolyPt[ i ].x - ptCenter.x;
		double dy = pPolyPt[ i ].y - ptCenter.y;
		double a = GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y );
		double angle = ( GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y ) * M_PI2 ) / 360.;
		double dist = sqrt( ( dx * dx ) + ( dy * dy ) ) + dMagnitude;

		pPolyPt[ i ].x = short( (double)ptCenter.x - ( cos( angle ) * dist ) );
		pPolyPt[ i ].y = short( (double)ptCenter.y + ( sin( angle ) * dist ) );

	}
}

XGRAPHIC_API void WINAPI InflatePolygon( const DPOINT& ptCenter, const UINT unptNum, 
									  DPOINT* pPolyPt, const double x, const double y )
{
	for ( UINT i = 0 ; i < unptNum ; i++ )
	{
		if ( pPolyPt[ i ].x < ptCenter.x ) pPolyPt[ i ].x -= x;
		else pPolyPt[ i ].x += x;

		if ( pPolyPt[ i ].y < ptCenter.y ) pPolyPt[ i ].y -= y;
		else pPolyPt[ i ].y += y;

	}
}

XGRAPHIC_API void WINAPI InflatePolygon( const DPOINT& ptCenter, const UINT unptNum, 
									  DPOINT* pPolyPt, const double dMagnitude )
{
	for ( UINT i = 0; i < unptNum; i++ )
	{
		double dx = pPolyPt[ i ].x - ptCenter.x;
		double dy = pPolyPt[ i ].y - ptCenter.y;
		double a = GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y );
		double angle = ( GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y ) * M_PI2 ) / 360.;
		double dist = sqrt( ( dx * dx ) + ( dy * dy ) ) + dMagnitude;

		pPolyPt[ i ].x = ptCenter.x - ( cos( angle ) * dist );
		pPolyPt[ i ].y = ptCenter.y + ( sin( angle ) * dist );
	}
}

XGRAPHIC_API void WINAPI DeflatePolygon( const POINT& ptCenter, const UINT unptNum, 
									  POINT* pPolyPt, const int x, const int y )
{
	for ( UINT i = 0 ; i < unptNum ; i++ )
	{
		if ( pPolyPt[ i ].x < ptCenter.x ) pPolyPt[ i ].x -= (short)-x;
		else pPolyPt[ i ].x += (short)-x;

		if ( pPolyPt[ i ].y < ptCenter.y ) pPolyPt[ i ].y -= (short)-y;
		else pPolyPt[ i ].y += (short)-y;

	}
}

XGRAPHIC_API void WINAPI DeflatePolygon( const POINT& ptCenter, const UINT unptNum, 
									  POINT* pPolyPt, const double dMagnitude )
{
	for ( UINT i = 0; i < unptNum; i++ )
	{
		double dx = pPolyPt[ i ].x - ptCenter.x;
		double dy = pPolyPt[ i ].y - ptCenter.y;
		double a = GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y );
		double angle = ( GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y ) * M_PI2 ) / 360.;
		double dist = sqrt( ( dx * dx ) + ( dy * dy ) ) - dMagnitude;

		pPolyPt[ i ].x = short( (double)ptCenter.x - ( cos( angle ) * dist ) );
		pPolyPt[ i ].y = short( (double)ptCenter.y + ( sin( angle ) * dist ) );

	}
}

XGRAPHIC_API void WINAPI DeflatePolygon( const DPOINT& ptCenter, const UINT unptNum, 
									  DPOINT* pPolyPt, const double x, const double y )
{
	for ( UINT i = 0 ; i < unptNum ; i++ )
	{
		if ( pPolyPt[ i ].x < ptCenter.x ) pPolyPt[ i ].x -= -x;
		else pPolyPt[ i ].x += -x;

		if ( pPolyPt[ i ].y < ptCenter.y ) pPolyPt[ i ].y -= -y;
		else pPolyPt[ i ].y += -y;

	}
}

XGRAPHIC_API void WINAPI DeflatePolygon( const DPOINT& ptCenter, const UINT unptNum, 
									  DPOINT* pPolyPt, const double dMagnitude )
{
	for ( UINT i = 0; i < unptNum; i++ )
	{
		double dx = pPolyPt[ i ].x - ptCenter.x;
		double dy = pPolyPt[ i ].y - ptCenter.y;
		double a = GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y );
		double angle = ( GetAngle( pPolyPt[ i ].x, pPolyPt[ i ].y, ptCenter.x, ptCenter.y ) * M_PI2 ) / 360.;
		double dist = sqrt( ( dx * dx ) + ( dy * dy ) ) - dMagnitude;

		pPolyPt[ i ].x = ptCenter.x - ( cos( angle ) * dist );
		pPolyPt[ i ].y = ptCenter.y + ( sin( angle ) * dist );
	}
}

XGRAPHIC_API BOOL WINAPI DPtInPolygon( const DPOINT & dpt, const UINT unptNum,
						  const DPOINT * pdPolyPt )
{
	int nCross = 0;
	const DPOINT * pdPolyPtCur = pdPolyPt+(unptNum-1);
	double dX1 = pdPolyPtCur->x - dpt.x;
	double dY1 = pdPolyPtCur->y - dpt.y;

	pdPolyPtCur = pdPolyPt;
	for ( UINT uC = 0; uC < unptNum; uC++ )
	{
		double dX2 = pdPolyPtCur->x - dpt.x;
		double dY2 = pdPolyPtCur->y - dpt.y;

		if ( (dY1 * dY2) < 0.0f )
		{
			double fInterValue = (dX2 - dX1) / (dY2 - dY1) * dY1;
			if ( fInterValue > dX1 )
			{
				nCross++;
			}
		}

		dX1 = dX2;
		dY1 = dY2;
		pdPolyPtCur++;
	}

	if ( (nCross & 1) == 1 )
	{
		return TRUE;
	}

	return FALSE;
}

XGRAPHIC_API BOOL WINAPI PtInPolygonW( const POINT & pt, const UINT uptNum,
						 const POINT * pPolyPt )
{
	HRGN hRgn = ::CreatePolygonRgn( pPolyPt, uptNum, WINDING );
	if ( hRgn == NULL ) return FALSE;

	BOOL bRet = ::PtInRegion( hRgn, pt.x, pt.y );

	::DeleteObject( hRgn );

	return bRet;
}

XGRAPHIC_API BOOL WINAPI PtInPolygon( const POINT & pt, const UINT uptNum,
						 const POINT * pPolyPt )
{
	int nCross = 0;
	const POINT * pPolyPtCur = pPolyPt+(uptNum-1);
	LONG lX1 = pPolyPtCur->x - pt.x;
	LONG lY1 = pPolyPtCur->y - pt.y;
	pPolyPtCur = pPolyPt;
	for ( UINT uC = 0; uC < uptNum; uC++ )
	{
		LONG lX2 = pPolyPtCur->x - pt.x;
		LONG lY2 = pPolyPtCur->y - pt.y;

		if ( (lY1 * lY2) < 0 )
		{
			double fInterValue = (double)(lX2 - lX1) / (double)(lY2 - lY1) * lY1;
			if ( fInterValue > (double)lX1 )
			{
				nCross++;
			}
		}

		lX1 = lX2;
		lY1 = lY2;
		pPolyPtCur++;
	}

	if ( (nCross & 1) == 1 )
	{
		return TRUE;
	}

	return FALSE;
}

XGRAPHIC_API BOOL WINAPI IsOverlapDBoundDLine( const DRECT & dbd, const DPOINT pdpt[2] )
{
	DRECT dbdLineBound;
	if ( pdpt[0].x < pdpt[1].x )
	{
		dbdLineBound.left = pdpt[0].x;
		dbdLineBound.right = pdpt[1].x;
	}
	else
	{
		dbdLineBound.left = pdpt[1].x;
		dbdLineBound.right = pdpt[0].x;
	}
	if ( pdpt[0].y < pdpt[1].y )
	{
		dbdLineBound.top = pdpt[0].y;
		dbdLineBound.bottom = pdpt[1].y;
	}
	else
	{
		dbdLineBound.top = pdpt[1].y;
		dbdLineBound.bottom = pdpt[0].y;
	}

	if ( dbd.left>dbdLineBound.right || dbd.right<dbdLineBound.left ||
		dbd.top>dbdLineBound.bottom || dbd.bottom<dbdLineBound.top ) return FALSE;

	double dDx = pdpt[0].x - pdpt[1].x;
	double dDy = pdpt[0].y - pdpt[1].y;

	double dP1 = dDy * ( dbd.left - pdpt[1].x ) - dDx * ( dbd.top - pdpt[1].y );
	double dP2 = dDy * ( dbd.left - pdpt[1].x ) - dDx * ( dbd.bottom - pdpt[1].y );
	double dP3 = dDy * ( dbd.right - pdpt[1].x ) - dDx * ( dbd.top - pdpt[1].y );
	double dP4 = dDy * ( dbd.right - pdpt[1].x ) - dDx * ( dbd.bottom - pdpt[1].y );

	if ( dP1 > 0.0f )
	{
		if ( dP2>0.0f && dP3>0.0f && dP4>0.0f ) return FALSE;
	}
	else if ( dP1 < 0.0f )
	{
		if ( dP2<0.0f && dP3<0.0f && dP4<0.0f ) return FALSE;
	}

	return TRUE;
}

XGRAPHIC_API BOOL WINAPI IsOverlapDBoundDBound( const DRECT & dbd1, const DRECT & dbd2 )
{
	if ( dbd1.left > dbd2.left )
	{
		if ( dbd1.left > dbd2.right ) return FALSE;
	}
	else
	{
		if ( dbd1.right < dbd2.left ) return FALSE;
	}

	if ( dbd1.top > dbd2.top )
	{
		if ( dbd1.top > dbd2.bottom ) return FALSE;
	}
	else
	{
		if ( dbd1.bottom < dbd2.top ) return FALSE;
	}

	return TRUE;
}

XGRAPHIC_API BOOL WINAPI IsContainDBoundDBound( const DRECT & dbd1, const DRECT & dbd2 )
{
	return dbd1.left <= dbd2.left && dbd1.right >= dbd2.right && dbd1.top <= dbd2.top && dbd1.bottom >= dbd2.bottom;
}

XGRAPHIC_API BOOL WINAPI IsOverlapDBoundDPoly( const DRECT & dbd, const UINT uNPtNum, const DPOINT * pdpt, BOOL bClosed )
{
	UINT uPC = 0;
	for ( ; uPC < uNPtNum-1; uPC++ )
	{
		if ( IsOverlapDBoundDLine( dbd, &pdpt[uPC] ) ) return TRUE;
	}

	if ( bClosed )
	{
		DPOINT dPt[2];
		dPt[0] = pdpt[uPC];
		dPt[1] = pdpt[0];
		if ( IsOverlapDBoundDLine(dbd, dPt) ) return TRUE;

		//if ( DPtInPolygon(*(CxDPoint*)&dbd, uNPtNum, pdpt) ) return TRUE;
	}

	return FALSE;
}

XGRAPHIC_API BOOL WINAPI GetPointOnPolyline( const double fRatio, const UINT uPtNum, const DPOINT * pdPts, DPOINT * pdPtOut )
{
	DPOINT * pPtPrv = (DPOINT*)pdPts;
	DPOINT * pPtCur;
	double dTotalDist = 0.;
	DPOINT * pPtGetCur = (DPOINT*)pdPts;
	DPOINT * pPtGetNext = (DPOINT*)pdPts;
	double dGetPntDistCur = 0.;
	double dGetPntDistNext = 0.;
	double dGetPntDist = 0.;
	for ( UINT ui = 1; ui < uPtNum; ui++ )
	{
		pPtCur = pPtPrv+1;
		double dx = pPtPrv->x - pPtCur->x;
		double dy = pPtPrv->y - pPtCur->y;
		dTotalDist += pow(dx*dx+dy*dy, 0.5);

		dGetPntDist = dTotalDist * fRatio;
		while ( dGetPntDist-dGetPntDistCur > dGetPntDistNext )
		{
			pPtGetCur = pPtGetNext;
			dGetPntDistCur += dGetPntDistNext;

			pPtGetNext++;
			dx = pPtGetNext->x - pPtGetCur->x;
			dy = pPtGetNext->y - pPtGetCur->y;
			dGetPntDistNext = pow(dx*dx+dy*dy, 0.5);
		}

		pPtPrv = pPtCur;
	}

	if ( dGetPntDistNext == 0. )
	{
		*pdPtOut = *pPtGetCur;
		return TRUE;
	}

	dGetPntDist -= dGetPntDistCur;
	dGetPntDist /= dGetPntDistNext;

	pdPtOut->x = (double)((pPtGetNext->x-pPtGetCur->x)*dGetPntDist + pPtGetCur->x);
	pdPtOut->y = (double)((pPtGetNext->y-pPtGetCur->y)*dGetPntDist + pPtGetCur->y);

	return TRUE;
}

XGRAPHIC_API BOOL WINAPI GetPointOnPolylineI( const double fRatio, const UINT uPtNum, const POINT * pPts, POINT * pPtOut )
{
	POINT * pPtPrv = (POINT*)pPts;
	POINT * pPtCur;
	double dTotalDist = 0.;
	POINT * pPtGetCur = (POINT*)pPts;
	POINT * pPtGetNext = (POINT*)pPts;
	double dGetPntDistCur = 0.;
	double dGetPntDistNext = 0.;
	double dGetPntDist = 0.;
	for ( UINT ui = 1; ui < uPtNum; ui++ )
	{
		pPtCur = pPtPrv+1;
		double dx = pPtPrv->x - pPtCur->x;
		double dy = pPtPrv->y - pPtCur->y;
		dTotalDist += pow(dx*dx+dy*dy, 0.5);

		dGetPntDist = dTotalDist * fRatio;
		while ( dGetPntDist-dGetPntDistCur > dGetPntDistNext )
		{
			pPtGetCur = pPtGetNext;
			dGetPntDistCur += dGetPntDistNext;

			pPtGetNext++;
			dx = pPtGetNext->x - pPtGetCur->x;
			dy = pPtGetNext->y - pPtGetCur->y;
			dGetPntDistNext = pow(dx*dx+dy*dy, 0.5);
		}

		pPtPrv = pPtCur;
	}

	if ( dGetPntDistNext == 0. )
	{
		*pPtOut = *pPtGetCur;
		return TRUE;
	}

	dGetPntDist -= dGetPntDistCur;
	dGetPntDist /= dGetPntDistNext;

	pPtOut->x = (long)((pPtGetNext->x-pPtGetCur->x)*dGetPntDist + pPtGetCur->x);
	pPtOut->y = (long)((pPtGetNext->y-pPtGetCur->y)*dGetPntDist + pPtGetCur->y);

	return TRUE;
}

XGRAPHIC_API BOOL WINAPI TrimPolylineI( UINT * puPtNum, POINT * pPts )
{
	POINT * pPtScan = pPts;
	POINT * pPtCur = pPts;
	UINT uPtNumGet = 0;
	UINT uTotal = *puPtNum;
	for ( UINT uC = 0; uC < uTotal; uC++ )
	{
		if ( pPtScan->x != pPtCur->x || pPtScan->y != pPtCur->y )
		{
			pPtCur++;
			*pPtCur = *pPtScan;

			uPtNumGet++;
		}

		pPtScan++;
	}
	if ( uTotal ) uPtNumGet++;
	*puPtNum = uPtNumGet;
	return TRUE;
}