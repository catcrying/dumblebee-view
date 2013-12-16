// xPointClipper.cpp: implementation of the CxPointClipper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XGraphic/xPointClipper.h>

#include <XGraphic/xGraphicPrimitive.h>
#include <float.h>

#include <XUtil/DebugSupport/xDebug.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CxPointClipper::ClipTop()
{
	if ( m_nCount == 0 ) return;

	int nPointClipped = GetClipPointCountTop();
	if ( nPointClipped==0 ) return;

	AllocateTempPointBuffer( nPointClipped );

	int nPtIndex = 0;
	DPOINT Point1, Point2;

	for (int i=0; i < m_nCount-1; i++)
	{
		if (m_lpDPoints[i].y <= m_rcBound.bottom )
		{
			m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[i];
		}

		if ( ((m_lpDPoints[i].y-m_rcBound.bottom)*(m_lpDPoints[i+1].y-m_rcBound.bottom)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].y = m_rcBound.bottom;
			Point1.x = m_lpDPoints[i].y;
			Point1.y = m_lpDPoints[i].x;
			Point2.x = m_lpDPoints[i+1].y;
			Point2.y = m_lpDPoints[i+1].x;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].x;
			GetCrossedEdge((const double)Point1.x, (const double)Point1.y, (const double)Point2.x, (const double)Point2.y, dCPY, (double)m_rcBound.bottom);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].x = (long)dCPY;
			nPtIndex++;
		}
	}

	if ( m_lpDPoints[m_nCount-1].y <= m_rcBound.bottom )
	{
		m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[m_nCount-1];
	}

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].y-m_rcBound.bottom)*(m_lpDPoints[0].y-m_rcBound.bottom)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].y = m_rcBound.bottom;
			Point1.x = m_lpDPoints[m_nCount-1].y;
			Point1.y = m_lpDPoints[m_nCount-1].x;
			Point2.x = m_lpDPoints[0].y;
			Point2.y = m_lpDPoints[0].x;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].x;
			GetCrossedEdge( (double)Point1.x, (double)Point1.y, (double)Point2.x, (double)Point2.y, dCPY, (double)m_rcBound.bottom);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].x = (long)dCPY;
		}
	}

	AllocatePointBuffer( nPointClipped );
	memcpy( m_lpDPoints, m_lpTempPointBuffer, nPointClipped*sizeof(CxDPoint) );
	m_nCount = nPointClipped;

}

void CxPointClipper::ClipBottom()
{
	if ( m_nCount == 0 ) return;

	int nPointClipped = GetClipPointCountBottom();
	if ( nPointClipped==0 ) return;

	AllocateTempPointBuffer( nPointClipped );

	int nPtIndex = 0;
	DPOINT Point1, Point2;

	for (int i=0; i<m_nCount-1; i++)
	{
		if (m_lpDPoints[i].y >= m_rcBound.top)
		{
			m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[i];
		}

		if ( ((m_lpDPoints[i].y-m_rcBound.top)*(m_lpDPoints[i+1].y-m_rcBound.top)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].y = m_rcBound.top;
			Point1.x = m_lpDPoints[i].y;
			Point1.y = m_lpDPoints[i].x;
			Point2.x = m_lpDPoints[i+1].y;
			Point2.y = m_lpDPoints[i+1].x;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].x;
			GetCrossedEdge((double)Point1.x, (double)Point1.y, (double)Point2.x, (double)Point2.y, dCPY, (double)m_rcBound.top);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].x = (long)dCPY;
			nPtIndex++;
		}
	}

	if ( m_lpDPoints[m_nCount-1].y >= m_rcBound.top )
	{
		m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[m_nCount-1];
	}

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].y-m_rcBound.top)*(m_lpDPoints[0].y-m_rcBound.top)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].y = m_rcBound.top;
			Point1.x = m_lpDPoints[m_nCount-1].y;
			Point1.y = m_lpDPoints[m_nCount-1].x;
			Point2.x = m_lpDPoints[0].y;
			Point2.y = m_lpDPoints[0].x;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].x;
			GetCrossedEdge((double)Point1.x, (double)Point1.y, (double)Point2.x, (double)Point2.y, dCPY, (double)m_rcBound.top);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].x = (long)dCPY;
		}
	}

	AllocatePointBuffer( nPointClipped );
	memcpy( m_lpDPoints, m_lpTempPointBuffer, nPointClipped*sizeof(CxDPoint) );
	m_nCount = nPointClipped;
}

void CxPointClipper::ClipLeft()
{
	if ( m_nCount == 0 ) return;
	
	int nPointClipped = GetClipPointCountLeft();

	if ( nPointClipped==0 ) return;

	AllocateTempPointBuffer( nPointClipped );

	int nPtIndex = 0;

	for (int i=0; i<m_nCount-1; i++)
	{
		if ( m_lpDPoints[i].x >= m_rcBound.left )
		{
			m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[i];
		}

		if ( ((m_lpDPoints[i].x-m_rcBound.left)*(m_lpDPoints[i+1].x-m_rcBound.left)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].x = m_rcBound.left;

			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].y;
			GetCrossedEdge((double)m_lpDPoints[i].x, (double)m_lpDPoints[i].y, (double)m_lpDPoints[i+1].x, (double)m_lpDPoints[i+1].y, dCPY, (double)m_rcBound.left);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].y = (long)dCPY;
			nPtIndex++;
		}
	}

	if ( m_lpDPoints[m_nCount-1].x >= m_rcBound.left )
	{
		m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[m_nCount-1];
	}

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].x-m_rcBound.left)*(m_lpDPoints[0].x-m_rcBound.left)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].x = m_rcBound.left;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].y;
			GetCrossedEdge((double)m_lpDPoints[m_nCount-1].x, (double)m_lpDPoints[m_nCount-1].y, (double)m_lpDPoints[0].x, (double)m_lpDPoints[0].y, dCPY, (double)m_rcBound.left);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].y = (long)dCPY;
		}
	}

	AllocatePointBuffer( nPointClipped );
	memcpy( m_lpDPoints, m_lpTempPointBuffer, nPointClipped*sizeof(CxDPoint) );
	m_nCount = nPointClipped;
}

void CxPointClipper::ClipRight()
{
	if ( m_nCount == 0 ) return;

	int nPointClipped = GetClipPointCountRight();
	if ( nPointClipped==0 ) return;

	AllocateTempPointBuffer( nPointClipped );

	int nPtIndex = 0;

	for (int i=0; i<m_nCount-1; i++)
	{
		if ( m_lpDPoints[i].x <= m_rcBound.right )
		{
			m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[i];
		}

		if ( ((m_lpDPoints[i].x-m_rcBound.right)*(m_lpDPoints[i+1].x-m_rcBound.right)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].x = m_rcBound.right;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].y;
			GetCrossedEdge((double)m_lpDPoints[i].x, (double)m_lpDPoints[i].y, (double)m_lpDPoints[i+1].x, (double)m_lpDPoints[i+1].y, dCPY, (double)m_rcBound.right);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].y = (long)dCPY;
			nPtIndex++;
		}
	}

	if ( m_lpDPoints[m_nCount-1].x <= m_rcBound.right )
	{
		m_lpTempPointBuffer[nPtIndex++] = m_lpDPoints[m_nCount-1];
	}

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].x-m_rcBound.right)*(m_lpDPoints[0].x-m_rcBound.right)) < 0 )
		{
			m_lpTempPointBuffer[nPtIndex].x = m_rcBound.right;
			double dCPY = (double)m_lpTempPointBuffer[nPtIndex].y;
			GetCrossedEdge((double)m_lpDPoints[m_nCount-1].x, (double)m_lpDPoints[m_nCount-1].y, (double)m_lpDPoints[0].x, (double)m_lpDPoints[0].y, dCPY, (double)m_rcBound.right);
			XASSERT( !_isnan(dCPY) );
			m_lpTempPointBuffer[nPtIndex].y = (long)dCPY;
		}
	}

	AllocatePointBuffer( nPointClipped );
	memcpy( m_lpDPoints, m_lpTempPointBuffer, nPointClipped*sizeof(CxDPoint) );
	m_nCount = nPointClipped;

}

int CxPointClipper::GetClipPointCountTop()
{
	if ( m_nCount == 0 ) return 0;

	int nRes = 0;

	for (int i=0; i < m_nCount-1; i++)
	{
		if ( m_lpDPoints[i].y <= m_rcBound.bottom ) nRes++;
		if (( (m_lpDPoints[i].y-m_rcBound.bottom) * (m_lpDPoints[i+1].y-m_rcBound.bottom)) < 0) nRes++;
	}

	if ( m_lpDPoints[m_nCount-1].y <= m_rcBound.bottom ) nRes++;

	if (m_bClosed)
	{
		if (( (m_lpDPoints[m_nCount-1].y-m_rcBound.bottom) * (m_lpDPoints[0].y-m_rcBound.bottom)) < 0 ) nRes++;
	}

	return nRes;
}

int CxPointClipper::GetClipPointCountBottom()
{
	if (m_nCount == 0) return 0;

	int nRes = 0;

	for (int i=0; i<m_nCount-1; i++)
	{
		if (m_lpDPoints[i].y >= m_rcBound.top ) nRes++;
		if (((m_lpDPoints[i].y-m_rcBound.top)*(m_lpDPoints[i+1].y-m_rcBound.top)) < 0 ) nRes++;
	}

	if (m_lpDPoints[m_nCount-1].y >= m_rcBound.top ) nRes++;

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].y-m_rcBound.top)*(m_lpDPoints[0].y-m_rcBound.top)) < 0 ) nRes++;
	}

	return nRes;
}

int CxPointClipper::GetClipPointCountLeft()
{
	if (m_nCount == 0) return 0;

	int nRes = 0;

	for (int i=0; i<m_nCount-1; i++)
	{
		if ( m_lpDPoints[i].x >= m_rcBound.left ) nRes++;
		if ( ((m_lpDPoints[i].x-m_rcBound.left)*(m_lpDPoints[i+1].x-m_rcBound.left)) < 0 ) nRes++;
	}

	if ( m_lpDPoints[m_nCount-1].x >= m_rcBound.left ) nRes++;

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].x-m_rcBound.left)*(m_lpDPoints[0].x-m_rcBound.left)) < 0 ) nRes++;
	}

	return nRes;
}

int CxPointClipper::GetClipPointCountRight()
{
	if (m_nCount == 0) return 0;

	int nRes=0;

	for (int i=0; i<m_nCount-1; i++)
	{
		if ( m_lpDPoints[i].x <= m_rcBound.right ) nRes++;
		if ( ((m_lpDPoints[i].x-m_rcBound.right)*(m_lpDPoints[i+1].x-m_rcBound.right)) < 0 ) nRes++;
	}

	if (m_lpDPoints[m_nCount-1].x <= m_rcBound.right ) nRes++;

	if (m_bClosed)
	{
		if ( ((m_lpDPoints[m_nCount-1].x-m_rcBound.right)*(m_lpDPoints[0].x-m_rcBound.right)) < 0 ) nRes++;
	}

	return nRes;
}

void CxPointClipper::GetCrossedEdge(const double dPtX1, const double dPtY1, const double dPtX2, const double dPtY2, 
								   double& dCrossedPointY, const double dX )
{
	double dDX = dPtX2 - dPtX1;
	XASSERT( !_isnan(dDX) );
	double dDY = dPtY2 - dPtY1;
	XASSERT( !_isnan(dDY) );
	double dRateX = ( dX - dPtX1 ) / dDX;
	XASSERT( !_isnan(dRateX) );

	dCrossedPointY = (double)( dDY * dRateX + dPtY1 );
	if ( _finite(dCrossedPointY) && !_isnan(dCrossedPointY) ) return;

	if ( !_finite(dDY) )
	{
		if ( !_finite(dDX) )
		{
			if ( !_finite(dPtY1) )
			{
				XASSERT( !_isnan(dPtY1) );
				dCrossedPointY = dPtY1;
			}
			else
			{
				XASSERT( !_isnan(dPtY2) );
				dCrossedPointY = dPtY2;
			}
		}
		else
		{
			if ( (dDX*dDY) > 0 )
			{
				XASSERT( !_isnan( ( dX - dPtX1 ) + dPtY1 ) );
				dCrossedPointY = ( dX - dPtX1 ) + dPtY1;
			}
			else
			{
				XASSERT( !_isnan( ( dPtX1 - dX ) + dPtY1 ) );
				dCrossedPointY = ( dPtX1 - dX ) + dPtY1;
			}
		}
	}
	else if ( !_finite(dPtX1) )
	{
		XASSERT( !_isnan(dPtY2) );
		dCrossedPointY = dPtY2;
	}
}

BOOL CxPointClipper::ClipPoints( DRECT& rcBound, const LPDPOINT lpPoints, int nCount, BOOL bClose/*=FALSE*/ )
{
	m_rcBound = rcBound;

//	XTRACE( _T("PointClipper: %d\r\n"), m_nBufferedCount );
	
	AllocatePointBuffer( nCount );

	memcpy( m_lpDPoints, lpPoints, sizeof(CxDPoint)*nCount );

	m_nCount = nCount;
	m_bClosed = bClose;
	
	if ( !CheckOutOfBound() ) 
	{
		return TRUE;
	}
	
	ClipRight();
	ClipBottom();
	ClipLeft();
	ClipTop();
	
	TrimPoints();
	
	return TRUE;
}

bool CxPointClipper::CheckOutOfBound() const
{
	int nInsideCnt = 0;
	CxDRect rcBound(m_rcBound);
	for ( int i=0 ; i<m_nCount ; i++ )
	{
		if ( !rcBound.PtInRect( m_lpDPoints[i] ) )
			return true;
	}
	
	return false;
}

void CxPointClipper::TrimPoints()
{
	LPDPOINT pPtScan = m_lpDPoints;
	LPDPOINT pPtCur = m_lpDPoints;
	int nPtNumGet = 0;
	int nTotal = m_nCount;

	for ( int nC = 0; nC < nTotal; nC++ )
	{
		if ( pPtScan->x != pPtCur->x || pPtScan->y != pPtCur->y )
		{
			pPtCur++;
			*pPtCur = *pPtScan;
			nPtNumGet++;
		}

		pPtScan++;
	}
	
	if ( nTotal ) nPtNumGet++;

	m_nCount = nPtNumGet;
}

void CxPointClipper::AllocatePointBuffer( int nCount )
{
	if ( !m_lpDPoints )
	{
		m_lpDPoints = new DPOINT[nCount];
		m_lpPoints = new POINT[nCount];
		m_nBufferedCount = nCount;
	}
	else
	{
		if ( m_nBufferedCount < nCount )
		{
			delete[] m_lpDPoints;
			delete[] m_lpPoints;
			m_lpDPoints = new DPOINT[nCount];
			m_lpPoints = new POINT[nCount];
			m_nBufferedCount = nCount;
		}
	}
}

void CxPointClipper::AllocateTempPointBuffer( int nCount )
{
	if ( !m_lpDPoints )
	{
		m_lpTempPointBuffer = new DPOINT[nCount];
		m_nTempBufferedCount = nCount;
	}
	else
	{
		if ( m_nTempBufferedCount < nCount )
		{
			delete[] m_lpTempPointBuffer;
			m_lpTempPointBuffer = new DPOINT[nCount];
			m_nTempBufferedCount = nCount;
		}
	}
}

const LPPOINT CxPointClipper::GetClipPoints()
{
	for ( int i=0 ; i<m_nCount ; i++ )
	{
		m_lpPoints[i].x	= (long)(m_lpDPoints[i].x+.5);
		m_lpPoints[i].y	= (long)(m_lpDPoints[i].y+.5);
	}

	return m_lpPoints;
}

CxPointClipper::~CxPointClipper()
{
	if ( m_lpPoints )
		delete[] m_lpPoints;
	if ( m_lpDPoints )
		delete[] m_lpDPoints;
	if ( m_lpTempPointBuffer )
		delete[] m_lpTempPointBuffer;
	m_lpTempPointBuffer = m_lpDPoints = NULL;
	m_lpPoints = NULL;
}