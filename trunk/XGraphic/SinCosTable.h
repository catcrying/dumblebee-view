/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#ifndef __SIN_COS_TABLE_H__
#define __SIN_COS_TABLE_H__

#include <math.h>

#ifndef M_PI
#define	M_PI		(3.1415926535897932384626433832795)
#endif

#define MAX_INDEX	100
class CSinCosTable
{
private:
	double m_pdSinTable[MAX_INDEX];
	double m_pdCosTable[MAX_INDEX];
	int		m_nIndexCount;
protected:
	void BuildTable( double dStepAngle )
	{
		double dAngleE = M_PI*2;
		double dAngleS = 0;
		
		double dGap = dAngleE - dAngleS;
		
		m_nIndexCount = (int)((dAngleE - dAngleS) / dStepAngle);
		if ( m_nIndexCount < 2 )
			m_nIndexCount = 2;
		
		//m_pdSinTable = new double[ m_nIndexCount+1 ];
		//m_pdCosTable = new double[ m_nIndexCount+1 ];
		double dInterval = (dAngleE - dAngleS) / m_nIndexCount;
		double dCurrentAngle;
		
		for ( int i = 0 ; i <= m_nIndexCount && i < MAX_INDEX ; i ++ )
		{
			dCurrentAngle = dInterval*i + dAngleS;
			
			m_pdSinTable[i] = sin(dCurrentAngle);
			m_pdCosTable[i] = cos(dCurrentAngle);
		}
		m_nIndexCount++;
	}
public:
	CSinCosTable() : m_nIndexCount(0)
	{
		BuildTable( M_PI / 180. * 10. );		// step: 5 degree
	}
	int GetTableIndexCount() { return m_nIndexCount; }
	double GetSinAt( int i ) { XASSERT(i>=0 && i<m_nIndexCount); return m_pdSinTable[i]; }
	double GetCosAt( int i ) { XASSERT(i>=0 && i<m_nIndexCount); return m_pdCosTable[i]; }
	~CSinCosTable() 
	{ 
	}
};

#endif