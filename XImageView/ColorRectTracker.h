/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#if !defined(AFX_COLORRECTTRACKER_H__3657A3C5_B27E_4E74_B245_3116F75EE9B0__INCLUDED_)
#define AFX_COLORRECTTRACKER_H__3657A3C5_B27E_4E74_B245_3116F75EE9B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CColorRectTracker : public CRectTracker
{
protected:
	CPen		m_WhiteBoldPen;
public:
	CColorRectTracker();
	virtual ~CColorRectTracker();
	
	void Draw(CDC* pDC);

};

#endif // !defined(AFX_COLORRECTTRACKER_H__3657A3C5_B27E_4E74_B245_3116F75EE9B0__INCLUDED_)
