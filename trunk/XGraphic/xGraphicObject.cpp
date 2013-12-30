// xGraphicObject.cpp: implementation of the CxGraphicObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <vector>
#include <map>

#include <XGraphic/xGraphicObject.h>
#include <XGraphic/xPointClipper.h>

#include <XUtil/xStopWatch.h>

#include <XGraphic/xGraphicPrimitive.h>
#include <XUtil/xCriticalSection.h>
#include <XUtil/String/xString.h>

#include <XGraphic/xArrowDrawer.h>

#include <math.h>
#include <malloc.h>
#include <atlconv.h>

#include "SinCosTable.h"

#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__ )
#endif

class CxBrushStyle
{
public:
	COLORREF	dwFgColor;
	COLORREF	dwBgColor;
	int			nStyle;

	const CxBrushStyle& operator = ( const CxBrushStyle& other );

	CxBrushStyle();
	~CxBrushStyle();
	CxBrushStyle( const CxBrushStyle& other );

    bool operator < ( const CxBrushStyle &OtherPenStyle );
};

bool operator < ( const CxBrushStyle& lhs, const CxBrushStyle& rhs );
bool operator == ( const CxBrushStyle& lhs, const CxBrushStyle& rhs );

class CxPenStyle
{
public:
	COLORREF	dwFgColor;
	COLORREF	dwBgColor;
	short		nThickness;
	int			nStyle;

	const CxPenStyle& operator = ( const CxPenStyle& other );

	CxPenStyle();
	~CxPenStyle();
	CxPenStyle( const CxPenStyle& other );

    bool operator < ( const CxPenStyle &OtherPenStyle );
};

bool operator < ( const CxPenStyle& lhs, const CxPenStyle& rhs );
bool operator == ( const CxPenStyle& lhs, const CxPenStyle& rhs );

struct GOPenCompare : public std::binary_function<CxPenStyle*, CxPenStyle*, bool>
{
	bool operator() ( const CxPenStyle* lhs, const CxPenStyle* rhs ) const;
};

struct GOBrushCompare : public std::binary_function<CxBrushStyle*, CxBrushStyle*, bool>
{
	bool operator() ( const CxBrushStyle* lhs, const CxBrushStyle* rhs ) const;
};
///////////////////////////////////////////////////////////////////////
//GOPenCompare
bool GOPenCompare::operator() ( const CxPenStyle* lhs, const CxPenStyle* rhs ) const
{
	return *lhs < *rhs;
}

///////////////////////////////////////////////////////////////////////
//GOBrushCompare
bool GOBrushCompare::operator() ( const CxBrushStyle* lhs, const CxBrushStyle* rhs ) const
{
	return *lhs < *rhs;
}

//////////////////////////////////////////////////////////////////////////
// CxGOTable
class CxGOTable
{
private:
	//typedef CMap<CxPenStyle*, CxPenStyle*&, HPEN, HPEN> PenStyleMapper;
	//typedef CMap<CxBrushStyle*, CxBrushStyle*&, HBRUSH, HBRUSH> BrushStyleMapper;
	typedef std::map<CxPenStyle*, HPEN, GOPenCompare> PenStyleMapper;
	typedef std::map<CxBrushStyle*, HBRUSH, GOBrushCompare> BrushStyleMapper;
	PenStyleMapper				m_PenStyleMapper;
	BrushStyleMapper			m_BrushStyleMapper;
	const UINT					m_nMaxTableSize;

private:
	CxGOTable();
	~CxGOTable();

	static CxGOTable* m_pThis;
	int m_nRef;
public:
	static CxGOTable* GetRef();
	static void ReleaseRef();

public:

	HPEN GetPen( CxPenStyle* pPenStyle );
	HBRUSH GetBrush( CxBrushStyle* pBrushStyle );
	void Clear();
};

CxGOTable* CxGOTable::GetRef()
{
	if (!m_pThis)
	{
		m_pThis = new CxGOTable();
		XTRACE( _T("CxGOTable Initialize\n") );
	}
	m_pThis->m_nRef++;
	XTRACE( _T("CxGOTable Ref: %d\n"), m_pThis->m_nRef );
	return m_pThis;
}

void CxGOTable::ReleaseRef()
{
	if (m_pThis)
	{
		m_pThis->m_nRef--;
		XTRACE( _T("CxGOTable UnRef: %d\n"), m_pThis->m_nRef );
		if (m_pThis->m_nRef <= 0)
		{
			delete m_pThis;
			m_pThis = NULL;
			XTRACE( _T("CxGOTable Finalize\n") );
		}
	}
}

CxGOTable* CxGOTable::m_pThis = NULL;
CxGOTable::CxGOTable() : m_nMaxTableSize(256), m_nRef(0)
{
}

CxGOTable::~CxGOTable() 
{
	Clear();
}

void CxGOTable::Clear()
{
	HPEN hPen;
	CxPenStyle* pPs;

	for (PenStyleMapper::iterator iter = m_PenStyleMapper.begin() ; iter != m_PenStyleMapper.end() ; iter++)
	{
		pPs = iter->first;
		hPen = iter->second;
		::DeleteObject( hPen );
		delete pPs;
	}

	m_PenStyleMapper.clear();

	HBRUSH hBrush;
	CxBrushStyle* pBs;
	for (BrushStyleMapper::iterator iter = m_BrushStyleMapper.begin() ; iter != m_BrushStyleMapper.end() ; iter++)
	{
		pBs = iter->first;
		hBrush = iter->second;
		::DeleteObject(hBrush);
		delete pBs;
	}

	m_BrushStyleMapper.clear();
}

HBRUSH CxGOTable::GetBrush( CxBrushStyle* pBrushStyle )
{
	if ( pBrushStyle == NULL ) return NULL;

	BrushStyleMapper::iterator iterFind = m_BrushStyleMapper.find(pBrushStyle);
	if (iterFind != m_BrushStyleMapper.end())
	{
		return iterFind->second;
	}
	
	if ( (UINT)m_BrushStyleMapper.size() > m_nMaxTableSize-1 )
	{
		return (HBRUSH)::GetStockObject( NULL_BRUSH );
	}

	LOGBRUSH logBrush;
	memset( &logBrush, 0, sizeof(LOGBRUSH) );
	logBrush.lbStyle = pBrushStyle->nStyle;
	logBrush.lbColor = pBrushStyle->dwFgColor;
	if ( pBrushStyle->nStyle == BS_HATCHED ) logBrush.lbHatch = HS_DIAGCROSS;
	HBRUSH hNewBrush = ::CreateBrushIndirect( &logBrush );
	
	CxBrushStyle* pNewBrushStyle = new CxBrushStyle;
	*pNewBrushStyle = *pBrushStyle;
	m_BrushStyleMapper.insert( BrushStyleMapper::value_type(pNewBrushStyle, hNewBrush) );
	
	XTRACE( _T("CreateNewBrush: %x\r\n"), hNewBrush );
	
	return hNewBrush;
}

HPEN CxGOTable::GetPen( CxPenStyle* pPenStyle )
{
	if ( pPenStyle == NULL ) return NULL;

	PenStyleMapper::iterator iterFind = m_PenStyleMapper.find(pPenStyle);
	if (iterFind != m_PenStyleMapper.end())
	{
		return iterFind->second;
	}
	
	if ( (UINT)m_PenStyleMapper.size() > m_nMaxTableSize-1 )
	{
		return (HPEN)::GetStockObject( BLACK_PEN );
	}

	HPEN hNewPen;
	if ( pPenStyle->nThickness == 1 || pPenStyle->nStyle == PS_SOLID )
	{
		hNewPen = ::CreatePen( pPenStyle->nStyle, pPenStyle->nThickness, pPenStyle->dwFgColor );
	}
	else
	{
		LOGBRUSH logBrush;
		logBrush.lbStyle = BS_SOLID;
		logBrush.lbColor = pPenStyle->dwFgColor;
		hNewPen = ::ExtCreatePen(PS_GEOMETRIC | pPenStyle->nStyle | PS_JOIN_ROUND,
			pPenStyle->nThickness, &logBrush, NULL, NULL);
	}
	
	CxPenStyle* pNewPenStyle = new CxPenStyle;
	*pNewPenStyle = *pPenStyle;
	m_PenStyleMapper.insert( PenStyleMapper::value_type(pNewPenStyle, hNewPen) );
	
	XTRACE( _T("CreateNewPen: %x\r\n"), hNewPen );
	
	return hNewPen;
}

//////////////////////////////////////////////////////////////////////////
// CxBrushStyle
const CxBrushStyle& CxBrushStyle::operator = ( const CxBrushStyle& other )
{
	dwFgColor = other.dwFgColor;
	dwBgColor = other.dwBgColor;
	nStyle = other.nStyle;
	return (*this);
}

CxBrushStyle::CxBrushStyle()  : dwFgColor(0), dwBgColor(0), nStyle(0) {}
CxBrushStyle::~CxBrushStyle() {}
CxBrushStyle::CxBrushStyle( const CxBrushStyle& other )
{
	dwFgColor = other.dwFgColor;
	dwBgColor = other.dwBgColor;
	nStyle = other.nStyle;
}

bool CxBrushStyle::operator < ( const CxBrushStyle &OtherPenStyle ) 
{ 	
	if ( dwFgColor < OtherPenStyle.dwFgColor ) return true;
	if ( dwFgColor > OtherPenStyle.dwFgColor ) return false;
	if ( nStyle < OtherPenStyle.nStyle ) return true;
	if ( nStyle > OtherPenStyle.nStyle ) return false;
	return true;
}

bool operator < ( const CxBrushStyle& lhs, const CxBrushStyle& rhs )
{
	if ( lhs.dwFgColor < rhs.dwFgColor ) return true;
	if ( lhs.dwFgColor > rhs.dwFgColor ) return false;
	if ( lhs.nStyle < rhs.nStyle ) return true;
	if ( lhs.nStyle > rhs.nStyle ) return false;
	
	return false;
}

bool operator == ( const CxBrushStyle& lhs, const CxBrushStyle& rhs )
{
	return (lhs.dwFgColor == rhs.dwFgColor) && (lhs.nStyle == rhs.nStyle);
}

//////////////////////////////////////////////////////////////////////////
// CxPenStyle
const CxPenStyle& CxPenStyle::operator = ( const CxPenStyle& other )
{
	dwFgColor = other.dwFgColor;
	dwBgColor = other.dwBgColor;
	nThickness = other.nThickness;
	nStyle = other.nStyle;
	return (*this);
}

CxPenStyle::CxPenStyle() : dwFgColor(0), dwBgColor(0), nThickness(0), nStyle(0) {}
CxPenStyle::~CxPenStyle() {}
CxPenStyle::CxPenStyle( const CxPenStyle& other )
{
	dwFgColor = other.dwFgColor;
	dwBgColor = other.dwBgColor;
	nThickness = other.nThickness;
	nStyle = other.nStyle;
}

bool CxPenStyle::operator < ( const CxPenStyle &OtherPenStyle ) 
{ 	
	if ( dwFgColor < OtherPenStyle.dwFgColor ) return true;
	if ( dwFgColor > OtherPenStyle.dwFgColor ) return false;
	if ( nThickness < OtherPenStyle.nThickness ) return true;
	if ( nThickness > OtherPenStyle.nThickness ) return false;
	if ( nStyle < OtherPenStyle.nStyle ) return true;
	if ( nStyle > OtherPenStyle.nStyle ) return false;
	return true;
}

bool operator < ( const CxPenStyle& lhs, const CxPenStyle& rhs )
{
	if ( lhs.dwFgColor < rhs.dwFgColor ) return true;
	if ( lhs.dwFgColor > rhs.dwFgColor ) return false;
	if ( lhs.nThickness < rhs.nThickness ) return true;
	if ( lhs.nThickness > rhs.nThickness ) return false;
	if ( lhs.nStyle < rhs.nStyle ) return true;
	if ( lhs.nStyle > rhs.nStyle ) return false;
	
	return false;
}

bool operator == ( const CxPenStyle& lhs, const CxPenStyle& rhs )
{
	return (lhs.dwFgColor == rhs.dwFgColor) && (lhs.nStyle == rhs.nStyle) && (lhs.nThickness == rhs.nThickness);
}

//////////////////////////////////////////////////////////////////////////
// CxGOBasic
CxGOBasic::GraphicObjectType CxGOBasic::GetType()
{
	return m_GraphicObjectType;
}
CxGOBasic::CxGOBasic() :
	m_GraphicObjectType(GraphicObjectTypeNone),
	m_pPenStyle(NULL), m_pBrushStyle(NULL),
	m_hBrush(NULL), m_hPen(NULL)
{
	m_pPenStyle = new CxPenStyle();
	m_pBrushStyle = new CxBrushStyle();
}
CxGOBasic::~CxGOBasic()
{
	delete m_pPenStyle;
	delete m_pBrushStyle;
}

const CxGOBasic& CxGOBasic::operator = ( const CxGOBasic& other )
{
	m_hPen = other.m_hPen;
	m_hBrush = other.m_hBrush;
	*m_pPenStyle = *other.m_pPenStyle;
	*m_pBrushStyle = *other.m_pBrushStyle;
	m_GraphicObjectType = other.m_GraphicObjectType;

	return (*this);
}
CxGOBasic::CxGOBasic( const CxGOBasic& other ) :
	m_GraphicObjectType(GraphicObjectTypeNone),
	m_pPenStyle(NULL), m_pBrushStyle(NULL),
	m_hBrush(NULL), m_hPen(NULL)
{
	m_hPen = other.m_hPen;
	m_hBrush = other.m_hBrush;
	if (!m_pPenStyle) m_pPenStyle = new CxPenStyle();
	if (!m_pBrushStyle) m_pBrushStyle = new CxBrushStyle();
	*m_pPenStyle = *other.m_pPenStyle;
	*m_pBrushStyle = *other.m_pBrushStyle;
	m_GraphicObjectType = other.m_GraphicObjectType;
}

//////////////////////////////////////////////////////////////////////////
// CxGOBox
void CxGOBox::CreateObject( COLORREF dwFgColor, int nLeft, int nTop, int nRight, int nBottom, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	rcAreaInt.left = nLeft; rcAreaInt.right = nRight; rcAreaInt.top = nTop; rcAreaInt.bottom = nBottom;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOBox::CreateObject( COLORREF dwFgColor, double dLeft, double dTop, double dRight, double dBottom, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	rcAreaDouble.left = dLeft; rcAreaDouble.right = dRight; rcAreaDouble.top = dTop; rcAreaDouble.bottom = dBottom;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGOBox::CreateObject( COLORREF dwFgColor, RECT rcArea, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, nStyle, nThickness, dwBgColor );
}

void CxGOBox::CreateObject( COLORREF dwFgColor, DRECT rcArea, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, nStyle, nThickness, dwBgColor );
}

void CxGOBox::NormalizeRect()
{
	if ( !bIsFloat )
	{
		int nT;
		if ( rcAreaInt.left > rcAreaInt.right ) { nT = rcAreaInt.right; rcAreaInt.right = rcAreaInt.left; rcAreaInt.left = nT; }
		if ( rcAreaInt.top > rcAreaInt.bottom ) { nT = rcAreaInt.bottom; rcAreaInt.bottom = rcAreaInt.top; rcAreaInt.top = nT; }
	}
	else
	{
		CxDRect rect(rcAreaDouble);
		rect.NormalizeRect();
		rcAreaDouble = rect;
	}
}

CxGOBox::CxGOBox() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeBox; }
CxGOBox::~CxGOBox() {}

class PolygonPointArray
{
protected:
	std::vector<POINT>	polygonPoints;
public:
	PolygonPointArray() {}
	~PolygonPointArray() {}
	void Add(POINT pt) { polygonPoints.push_back(pt); }
	void Clear() { polygonPoints.clear(); }
	int Count() { return (int)polygonPoints.size(); }
	BOOL IsEmpty() { return polygonPoints.empty() ? TRUE : FALSE; }
	const PolygonPointArray& operator = ( const PolygonPointArray& Other )
	{
		polygonPoints.clear();
		polygonPoints.resize( Other.polygonPoints.size() );
		std::copy( Other.polygonPoints.begin(), Other.polygonPoints.end(), polygonPoints.begin() );
	}
	const POINT& operator[] (int index) const
	{
		return polygonPoints[index];
	}
	POINT& operator[] (int index)
	{
		return polygonPoints.at(index);
	}
	POINT& GetAt(int index)
	{
		return polygonPoints.at(index);
	}
};

class PolygonDPointArray
{
protected:
	std::vector<DPOINT>	polygonPoints;
public:
	PolygonDPointArray() {}
	~PolygonDPointArray() {}
	void Add(DPOINT pt) { polygonPoints.push_back(pt); }
	void Clear() { polygonPoints.clear(); }
	int Count() { return (int)polygonPoints.size(); }
	BOOL IsEmpty() { return polygonPoints.empty() ? TRUE : FALSE; }
	const PolygonDPointArray& operator = ( const PolygonDPointArray& Other )
	{
		polygonPoints.clear();
		polygonPoints.resize( Other.polygonPoints.size() );
		std::copy( Other.polygonPoints.begin(), Other.polygonPoints.end(), polygonPoints.begin() );
		return (*this);
	}
	const DPOINT& operator[] (int index) const
	{
		return polygonPoints[index];
	}
	DPOINT& operator[] (int index)
	{
		return polygonPoints.at(index);
	}
	DPOINT& GetAt(int index)
	{
		return polygonPoints.at(index);
	}
};

//////////////////////////////////////////////////////////////////////////
// CxGOEllipse
CxGOEllipse::CxGOEllipse( const CxGOEllipse& Other )
{
	m_pPolygonDPointArray = new PolygonDPointArray();
	bIsFloat		= Other.bIsFloat;	
	*m_pPenStyle	= *Other.m_pPenStyle;
	m_hPen			= Other.m_hPen;
	m_GraphicObjectType		= Other.m_GraphicObjectType;
	rcAreaInt		= Other.rcAreaInt;
	rcAreaDouble	= Other.rcAreaDouble;
	nIndex			= Other.nIndex;
	*m_pPolygonDPointArray = *Other.m_pPolygonDPointArray;
}

CxGOEllipse::CxGOEllipse() : bIsFloat(FALSE)
{
	m_pPolygonDPointArray = new PolygonDPointArray();
	m_GraphicObjectType = GraphicObjectTypeEllipse;
}
CxGOEllipse::~CxGOEllipse()
{
	delete m_pPolygonDPointArray;
}

BOOL CxGOEllipse::IsPolygon() { return m_pPolygonDPointArray->IsEmpty() ? FALSE : TRUE; }

void CxGOEllipse::CreateObject( COLORREF dwFgColor, int nLeft, int nTop, int nRight, int nBottom, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	rcAreaInt.left = nLeft; rcAreaInt.right = nRight; rcAreaInt.top = nTop; rcAreaInt.bottom = nBottom;
	
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOEllipse::CreateObject( COLORREF dwFgColor, double dLeft, double dTop, double dRight, double dBottom, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	rcAreaDouble.left = dLeft; rcAreaDouble.right = dRight; rcAreaDouble.top = dTop; rcAreaDouble.bottom = dBottom;
	
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGOEllipse::CreateObject( COLORREF dwFgColor, RECT rcArea, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, nStyle, nThickness, dwBgColor );
}
void CxGOEllipse::CreateObject( COLORREF dwFgColor, DRECT rcArea, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, rcArea.left, rcArea.top, rcArea.right, rcArea.bottom, nStyle, nThickness, dwBgColor );
}
void CxGOEllipse::NormalizeRect()
{
	int nT;
	if ( !bIsFloat )
	{
		if ( rcAreaInt.left > rcAreaInt.right ) { nT = rcAreaInt.right; rcAreaInt.right = rcAreaInt.left; rcAreaInt.left = nT; }
		if ( rcAreaInt.top > rcAreaInt.bottom ) { nT = rcAreaInt.bottom; rcAreaInt.bottom = rcAreaInt.top; rcAreaInt.top = nT; }
	}
	else
	{
		CxDRect rcArea(rcAreaDouble);
		rcArea.NormalizeRect();
		rcAreaDouble = rcArea;
	}
}

const CxGOEllipse& CxGOEllipse::operator = ( const CxGOEllipse& Other )
{
	bIsFloat		= Other.bIsFloat;
	*m_pPenStyle	= *Other.m_pPenStyle;
	m_hPen			= Other.m_hPen;
	m_GraphicObjectType		= Other.m_GraphicObjectType;
	rcAreaInt		= Other.rcAreaInt;
	rcAreaDouble	= Other.rcAreaDouble;
	nIndex			= Other.nIndex;
	*m_pPolygonDPointArray = *Other.m_pPolygonDPointArray;		
	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// CxGOLine
void CxGOLine::CreateObject( COLORREF dwFgColor, int x1, int y1, int x2, int y2, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptInt[0].x = x1; ptInt[0].y = y1; ptInt[1].x = x2; ptInt[1].y = y2;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOLine::CreateObject( COLORREF dwFgColor, double x1, double y1, double x2, double y2, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptDouble[0].x = x1; ptDouble[0].y = y1; ptDouble[1].x = x2; ptDouble[1].y = y2;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGOLine::CreateObject( COLORREF dwFgColor, POINT pt1, POINT pt2, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt1.x, pt1.y, pt2.x, pt2.y, nStyle, nThickness, dwBgColor );
}
void CxGOLine::CreateObject( COLORREF dwFgColor, DPOINT pt1, DPOINT pt2, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt1.x, pt1.y, pt2.x, pt2.y, nStyle, nThickness, dwBgColor );
}

CxGOLine::CxGOLine() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeLine; }
CxGOLine::~CxGOLine() {}

//////////////////////////////////////////////////////////////////////////
// CxGOArrow
void CxGOArrow::CreateObject( COLORREF dwFgColor, int x1, int y1, int x2, int y2, 
							ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle/*=PS_SOLID*/ )
{
	ptInt[0].x = x1; ptInt[0].y = y1; ptInt[1].x = x2; ptInt[1].y = y2;
	direction = arrowDirection;
	headType = arrowType;
	size = nArrowSize;
	m_pPenStyle->dwBgColor = -1;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nLineStyle;
	m_pPenStyle->nThickness = 1;
	m_pBrushStyle->dwFgColor = dwFgColor;
	m_pBrushStyle->nStyle = BS_SOLID;
	bIsFloat = FALSE;
}
void CxGOArrow::CreateObject( COLORREF dwFgColor, double x1, double y1, double x2, double y2, 
							ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle/*=PS_SOLID*/ )
{
	ptDouble[0].x = x1; ptDouble[0].y = y1; ptDouble[1].x = x2; ptDouble[1].y = y2;
	direction = arrowDirection;
	headType = arrowType;
	size = nArrowSize;
	m_pPenStyle->dwBgColor = -1;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nLineStyle;
	m_pPenStyle->nThickness = 1;
	m_pBrushStyle->dwFgColor = dwFgColor;
	m_pBrushStyle->nStyle = BS_SOLID;
	bIsFloat = TRUE;
}
void CxGOArrow::CreateObject( COLORREF dwFgColor, POINT pt1, POINT pt2, 
							ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle/*=PS_SOLID*/ )
{
	CreateObject( dwFgColor, pt1.x, pt1.y, pt2.x, pt2.y, arrowDirection, arrowType, nArrowSize, nLineStyle );
}
void CxGOArrow::CreateObject( COLORREF dwFgColor, DPOINT pt1, DPOINT pt2, 
							ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle/*=PS_SOLID*/ )
{
	CreateObject( dwFgColor, pt1.x, pt1.y, pt2.x, pt2.y, arrowDirection, arrowType, nArrowSize, nLineStyle );
}
CxGOArrow::CxGOArrow() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeArrow; }
CxGOArrow::~CxGOArrow() {}

//////////////////////////////////////////////////////////////////////////
// CxGODCross
void CxGODCross::CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptInt.x = x; ptInt.y = y;
	this->nLength = nLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGODCross::CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptDouble.x = x; ptDouble.y = y;
	this->nLength = nLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGODCross::CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nStyle, nThickness, dwBgColor );
}
void CxGODCross::CreateObject( COLORREF dwFgColor, DPOINT pt, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nStyle, nThickness, dwBgColor );
}
CxGODCross::CxGODCross() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeDiagonalCross; }
CxGODCross::~CxGODCross() {}

//////////////////////////////////////////////////////////////////////////
// CxGOAlignMark
void CxGOAlignMark::CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nMarkLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptInt.x = x; ptInt.y = y;
	this->nLength		= nLength;
	this->nMarkLength = nMarkLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOAlignMark::CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nMarkLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptDouble.x = x; ptDouble.y = y;
	this->nLength		= nLength;
	this->nMarkLength = nMarkLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGOAlignMark::CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nMarkLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nMarkLength, nStyle, nThickness, dwBgColor );
}
void CxGOAlignMark::CreateObject( COLORREF dwFgColor, DPOINT pt, int nLength, int nMarkLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nMarkLength, nStyle, nThickness, dwBgColor );
}

CxGOAlignMark::CxGOAlignMark() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeAlignMark; }
CxGOAlignMark::~CxGOAlignMark() {}

//////////////////////////////////////////////////////////////////////////
// CxGOCross
void CxGOCross::CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptInt.x = x; ptInt.y = y;
	this->nLength = nLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOCross::CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	ptDouble.x = x; ptDouble.y = y;
	this->nLength = nLength;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}
void CxGOCross::CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nStyle, nThickness, dwBgColor );
}
void CxGOCross::CreateObject( COLORREF dwFgColor, CxDPoint pt, int nLength, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nLength, nStyle, nThickness, dwBgColor );
}

CxGOCross::CxGOCross() : bIsFloat(FALSE) { m_GraphicObjectType = GraphicObjectTypeCross; }
CxGOCross::~CxGOCross() {}

//////////////////////////////////////////////////////////////////////////
// CxGOPoint
void CxGOPoint::CreateObject( COLORREF dwFgColor, int x, int y )
{
	ptInt.x = x; ptInt.y = y;
	m_pPenStyle->dwBgColor = -1;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = PS_SOLID;
	m_pPenStyle->nThickness = 1;
	m_pBrushStyle->dwFgColor = dwFgColor;
	m_pBrushStyle->nStyle = BS_HATCHED;
}
void CxGOPoint::CreateObject( COLORREF dwFgColor, POINT pt )
{
	CreateObject( dwFgColor, pt.x, pt.y );
}
CxGOPoint::CxGOPoint() { m_GraphicObjectType = GraphicObjectTypePoint; }
CxGOPoint::~CxGOPoint() {}

//////////////////////////////////////////////////////////////////////////
// CxGOText
void CxGOText::CreateObject( COLORREF dwFgColor, int x, int y, int nHeight, BOOL bDynamic/*=FALSE*/, TextAlignment eStyle/*=TextAlignmentRight*/, COLORREF dwBgColor /*= -1*/ )
{
	pt.x = x; pt.y = y;
	eTextAlignment = eStyle;
	this->nHeight = nHeight;
	this->bDynamic = bDynamic;
	this->dwFgColor = dwFgColor;
	this->dwBgColor = dwBgColor;
}
void CxGOText::CreateObject( COLORREF dwFgColor, POINT pt, int nHeight, BOOL bDynamic/*=FALSE*/, TextAlignment eStyle/*=TextAlignmentRight*/, COLORREF dwBgColor /*= -1*/ )
{
	CreateObject( dwFgColor, pt.x, pt.y, nHeight, bDynamic, eStyle, dwBgColor );
}

void CxGOText::SetText( LPCTSTR lpszFormat, ... )
{
	USES_CONVERSION;
	va_list argList;
	va_start( argList, lpszFormat );
	TCHAR lpszBuffer[256];
	XVERIFY(_vsntprintf_s(lpszBuffer, 256, lpszFormat, argList) <= 256 );
	WCHAR* pwszBuffer = T2W(lpszBuffer);
	memset( wszText, 0, sizeof(WCHAR)*256 );
	memcpy( wszText, T2W(lpszBuffer), sizeof(WCHAR)*lstrlenW(pwszBuffer) );
	va_end( argList );
}

CxGOText::CxGOText() : bDynamic(FALSE) { m_GraphicObjectType = GraphicObjectTypeText; }
CxGOText::~CxGOText() {}

//////////////////////////////////////////////////////////////////////////
// CxGOPolygon
void CxGOPolygon::CreateObject( COLORREF dwFgColor, POINT* lptPolygon, int nPtCnt, BOOL bClosed /*= TRUE*/, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	m_pPolygonPointArray->Clear();
	for ( int i=0 ; i<nPtCnt ; i++ )
		m_pPolygonPointArray->Add( lptPolygon[i] );
	this->bClosed = bClosed;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = FALSE;
}
void CxGOPolygon::CreateObject( COLORREF dwFgColor, DPOINT* lptPolygon, int nPtCnt, BOOL bClosed /*= TRUE*/, int nStyle /*= PS_SOLID*/, int nThickness/*=1*/, COLORREF dwBgColor/*=-1*/ )
{
	m_pPolygonDPointArray->Clear();
	for ( int i=0 ; i<nPtCnt ; i++ )
		m_pPolygonDPointArray->Add( lptPolygon[i] );

	this->bClosed = bClosed;
	m_pPenStyle->dwBgColor = dwBgColor;
	m_pPenStyle->dwFgColor = dwFgColor;
	m_pPenStyle->nStyle = nStyle;
	m_pPenStyle->nThickness = nThickness;
	bIsFloat = TRUE;
}

CxGOPolygon::CxGOPolygon() : bIsFloat(FALSE)
{ 
	m_pPolygonPointArray = new PolygonPointArray();
	m_pPolygonDPointArray = new PolygonDPointArray();
	m_GraphicObjectType = GraphicObjectTypePolygon;
}
CxGOPolygon::~CxGOPolygon()
{
	delete m_pPolygonPointArray;
	delete m_pPolygonDPointArray;
}

static CSinCosTable s_SinCosTable;

void CxGOEllipse::ToPolygon()
{
	m_pPolygonDPointArray->Clear();
	
	CxDRect rect;
	if ( bIsFloat )
		rect = rcAreaDouble;
	else
		rect = rcAreaInt;
	
	CxDPoint ptCenter = rect.CenterPoint();
	
	int nSplitCount = s_SinCosTable.GetTableIndexCount();
	
	double a = rect.Width()/2;
	double b = rect.Height()/2;
	
	CxDPoint ptPoly;
	for ( int i = 0 ; i < nSplitCount ; i ++ )
	{
		double dSinT = s_SinCosTable.GetSinAt(i);
		double dCosT = s_SinCosTable.GetCosAt(i);
		
		ptPoly.x = (dCosT * a) + ptCenter.x; 
		ptPoly.y = (dSinT * b) + ptCenter.y;
		
		m_pPolygonDPointArray->Add( ptPoly );
		//pDC->Rectangle( CRect(ptVector.x-2, ptVector.y-2, ptVector.x+2, ptVector.y+2) );
	}
}


class CxObjectGenerator
{
public:
	template< class TTable, class TArray >
	static void DeleteObjects( TTable& _Table, TArray& _Array )
	{
		HPEN hPen;
		TArray* pTArray;
		for (TTable::iterator iter = _Table.begin() ; iter != _Table.end() ; iter++)
		{
			hPen = iter->first;
			pTArray = iter->second;
			delete pTArray;
		}
		_Table.clear();
	}
	template< class TTable, class TArray >
	static void GenerateObjects( CxGOTable* pGOTable, TTable& _Table, TArray& _Array )
	{		
		HPEN hPen;
		TArray* pTArray = NULL;
		for (TTable::iterator iter = _Table.begin() ; iter != _Table.end() ; iter++)
		{
			hPen = iter->first;
			pTArray = iter->second;
			delete pTArray;
		}
		_Table.clear();

		for ( int i=0 ; i<(int)_Array.size() ; i++ )
		{	
			if ( _Array[i].GetType() == CxGOBasic::GraphicObjectTypePoint || _Array[i].GetType() == CxGOBasic::GraphicObjectTypeArrow )
			{
				_Array[i].m_hBrush = pGOTable->GetBrush( _Array[i].m_pBrushStyle );
			}
			_Array[i].m_hPen = pGOTable->GetPen( _Array[i].m_pPenStyle );
			
			TArray* pIArray = NULL;
			TTable::iterator iterFind = _Table.find( _Array[i].m_hPen );
			if ( iterFind != _Table.end() )
			{
				pIArray = iterFind->second;
			}
			else
			{
				TArray* pNewArray = new TArray;
				_Table.insert( TTable::value_type(_Array[i].m_hPen, pNewArray) );
				pIArray = pNewArray;
			}

			XASSERT( pIArray );
			pIArray->push_back( _Array[i] );
		}
	}
};

class CxGraphicObjectData
{
protected:
	CxGOTable*	m_pGOTable;
public:
	explicit CxGraphicObjectData(CxGOTable* pGOTable) : m_pGOTable(pGOTable)
	{
	}
public:
	struct LayerProperty
	{
		BOOL	bVisible;
		float	fZoomMin;
		float	fZoomMax;
		LayerProperty( BOOL _bVisible, float _fZoomMin, float _fZoomMax );
		LayerProperty();
	};

	LayerProperty		m_LayerProperty[MAX_LAYER];

	typedef std::vector<COLORBOX>						COLORBOX_ARRAY;
	typedef std::vector<COLORLINE>						COLORLINE_ARRAY;
	typedef std::vector<COLORTEXT>						COLORTEXT_ARRAY;
	typedef std::vector<COLORCROSS>						COLORCROSS_ARRAY;
	typedef std::vector<COLORDCROSS>					COLORDCROSS_ARRAY;
	typedef std::vector<COLORELLIPSE>					COLORELLIPSE_ARRAY;
	typedef std::vector<COLORPOINT>						COLORPOINT_ARRAY;
	typedef std::vector<COLORARROW>						COLORARROW_ARRAY;
	typedef std::vector<COLORPOLYGON>					COLORPOLYGON_ARRAY;
	typedef std::vector<COLORALIGNMARK>					COLORALIGNMARK_ARRAY;

	typedef std::map<HPEN, COLORBOX_ARRAY*>				TableGOBox;
	typedef std::map<HPEN, COLORLINE_ARRAY*>			TableGOLine;
	typedef std::map<HPEN, COLORCROSS_ARRAY*>			TableGOCross;
	typedef std::map<HPEN, COLORDCROSS_ARRAY*>			TableGODCross;
	typedef std::map<HPEN, COLORELLIPSE_ARRAY*>			TableGOEllipse;
	typedef std::map<HPEN, COLORPOINT_ARRAY*>			TableGOPoint;
	typedef std::map<HPEN, COLORARROW_ARRAY*>			TableGOArrow;
	typedef std::map<HPEN, COLORPOLYGON_ARRAY*>			TableGOPolygon;
	typedef std::map<HPEN, COLORALIGNMARK_ARRAY*>		TableGOAlignMark;

	TableGOBox			m_TableGOBox[MAX_LAYER];
	TableGOLine			m_TableGOLine[MAX_LAYER];
	TableGOCross		m_TableGOCross[MAX_LAYER];
	TableGODCross		m_TableGODCross[MAX_LAYER];
	TableGOEllipse		m_TableGOEllipse[MAX_LAYER];
	TableGOPoint		m_TableGOPoint[MAX_LAYER];
	TableGOArrow		m_TableGOArrow[MAX_LAYER];
	TableGOPolygon		m_TableGOPolygon[MAX_LAYER];
	TableGOAlignMark	m_TableGOAlignMark[MAX_LAYER];

	BOOL				m_bRebuildGOBox[MAX_LAYER];
	BOOL				m_bRebuildGOLine[MAX_LAYER];
	BOOL				m_bRebuildGOCross[MAX_LAYER];
	BOOL				m_bRebuildGODCross[MAX_LAYER];
	BOOL				m_bRebuildGOEllipse[MAX_LAYER];
	BOOL				m_bRebuildGOPoint[MAX_LAYER];
	BOOL				m_bRebuildGOArrow[MAX_LAYER];
	BOOL				m_bRebuildGOPolygon[MAX_LAYER];
	BOOL				m_bRebuildGOAlignMark[MAX_LAYER];
	
	COLORBOX_ARRAY			m_BoxArray[MAX_LAYER];
	COLORLINE_ARRAY			m_LineArray[MAX_LAYER];
	COLORTEXT_ARRAY			m_TextArray[MAX_LAYER];
	COLORCROSS_ARRAY		m_CrossArray[MAX_LAYER];
	COLORDCROSS_ARRAY		m_DCrossArray[MAX_LAYER];
	COLORELLIPSE_ARRAY		m_EllipseArray[MAX_LAYER];
	COLORPOINT_ARRAY		m_PointArray[MAX_LAYER];
	COLORARROW_ARRAY		m_ArrowArray[MAX_LAYER];
	COLORPOLYGON_ARRAY		m_PolygonArray[MAX_LAYER];
	COLORALIGNMARK_ARRAY	m_AlignMarkArray[MAX_LAYER];

	int m_nActiveBoxIndex[MAX_LAYER];
	int m_nActiveLineIndex[MAX_LAYER];
	int m_nActiveTextIndex[MAX_LAYER];
	int m_nActiveCrossIndex[MAX_LAYER];
	int m_nActiveDCrossIndex[MAX_LAYER];
	int m_nActiveEllipseIndex[MAX_LAYER];
	int m_nActivePointIndex[MAX_LAYER];
	int m_nActiveArrowIndex[MAX_LAYER];
	int m_nActivePolygonIndex[MAX_LAYER];
	int m_nActiveAlignMarkIndex[MAX_LAYER];

	void DeleteObjects()
	{
		for ( int i=0 ; i<MAX_LAYER ; i++ )
		{
			CxObjectGenerator::DeleteObjects( m_TableGOBox[i],			m_BoxArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOCross[i],		m_CrossArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGODCross[i],		m_DCrossArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOLine[i],			m_LineArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOPoint[i],		m_PointArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOArrow[i],		m_ArrowArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOPolygon[i],		m_PolygonArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOAlignMark[i],	m_AlignMarkArray[i] );
			CxObjectGenerator::DeleteObjects( m_TableGOEllipse[i],		m_EllipseArray[i] );
		}
	}
	void Clear()
	{
		for ( int i=0 ; i<MAX_LAYER ; i++ )
		{
			m_BoxArray[i].clear();
			m_LineArray[i].clear();
			m_ArrowArray[i].clear();
			m_TextArray[i].clear();
			m_CrossArray[i].clear();
			m_DCrossArray[i].clear();
			m_EllipseArray[i].clear();
			m_PointArray[i].clear();
			m_PolygonArray[i].clear();
			m_AlignMarkArray[i].clear();
			m_nActiveBoxIndex[i]		= -1;
			m_nActiveLineIndex[i]		= -1;
			m_nActiveTextIndex[i]		= -1;
			m_nActiveCrossIndex[i]		= -1;
			m_nActiveDCrossIndex[i]		= -1;
			m_nActiveEllipseIndex[i]	= -1;
			m_nActivePointIndex[i]		= -1;
			m_nActiveArrowIndex[i]		= -1;
			m_nActivePolygonIndex[i]	= -1;
			m_nActiveAlignMarkIndex[i]	= -1;
		}
	}
	void Init()
	{
		for ( int i=0 ; i<MAX_LAYER ; i++ )
		{
			m_nActiveBoxIndex[i]		= -1;
			m_nActiveLineIndex[i]		= -1;
			m_nActiveTextIndex[i]		= -1;
			m_nActiveCrossIndex[i]		= -1;
			m_nActiveDCrossIndex[i]		= -1;
			m_nActiveEllipseIndex[i]	= -1;
			m_nActivePointIndex[i]		= -1;
			m_nActiveArrowIndex[i]		= -1;
			m_nActivePolygonIndex[i]	= -1;
			m_nActiveAlignMarkIndex[i]	= -1;

			m_bRebuildGOBox[i]			= FALSE;
			m_bRebuildGOLine[i]			= FALSE;
			m_bRebuildGOCross[i]		= FALSE;
			m_bRebuildGODCross[i]		= FALSE;
			m_bRebuildGOEllipse[i]		= FALSE;
			m_bRebuildGOPoint[i]		= FALSE;
			m_bRebuildGOArrow[i]		= FALSE;
			m_bRebuildGOPolygon[i]		= FALSE;
			m_bRebuildGOAlignMark[i]	= FALSE;

			m_LayerProperty[i]			= LayerProperty(TRUE, FLT_MIN, FLT_MAX);
		}
	}
	void GenerateEllipse(int nLayer)
	{
		HPEN hPen;
		COLORELLIPSE_ARRAY* pEllipseArray;
		for ( TableGOEllipse::iterator iter = m_TableGOEllipse[nLayer].begin(); iter != m_TableGOEllipse[nLayer].end(); iter++ )
		{
			hPen = iter->first;
			pEllipseArray = iter->second;
			delete pEllipseArray;
		}

		m_TableGOEllipse[nLayer].clear();

		for ( int i=0 ; i<(int)m_EllipseArray[nLayer].size() ; i++ )
		{
			CxGOEllipse& GOEllipse = m_EllipseArray[nLayer].at( i );
			GOEllipse.m_hPen = m_pGOTable->GetPen( GOEllipse.m_pPenStyle );

			COLORELLIPSE_ARRAY* pArrayEllipse = NULL;
			TableGOEllipse::iterator iterFind = m_TableGOEllipse[nLayer].find( GOEllipse.m_hPen );
			if ( iterFind != m_TableGOEllipse[nLayer].end() )
			{
				pArrayEllipse = iterFind->second;
			}
			else
			{
				COLORELLIPSE_ARRAY* pNewArray = new COLORELLIPSE_ARRAY;
				m_TableGOEllipse[nLayer].insert( TableGOEllipse::value_type(GOEllipse.m_hPen, pNewArray) );
				pArrayEllipse = pNewArray;
			}

			if ( !GOEllipse.IsPolygon() ) GOEllipse.ToPolygon();

			XASSERT( pArrayEllipse );
			pArrayEllipse->push_back( GOEllipse );
		}
	}
	void GenerateObjects()
	{
		for ( int i=MAX_LAYER-1 ; i>=0 ; i-- )
		{
			if ( !m_LayerProperty[i].bVisible ) continue;
		
			if ( m_bRebuildGOBox[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOBox[i], m_BoxArray[i] );
				m_bRebuildGOBox[i] = FALSE;
			}
			if ( m_bRebuildGOCross[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOCross[i], m_CrossArray[i] );
				m_bRebuildGOCross[i] = FALSE;
			}
			if ( m_bRebuildGODCross[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGODCross[i], m_DCrossArray[i] );
				m_bRebuildGODCross[i] = FALSE;
			}
			if ( m_bRebuildGOLine[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOLine[i], m_LineArray[i] );
				m_bRebuildGOLine[i] = FALSE;
			}
			if ( m_bRebuildGOPoint[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOPoint[i], m_PointArray[i] );
				m_bRebuildGOPoint[i] = FALSE;
			}
			if ( m_bRebuildGOArrow[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOArrow[i], m_ArrowArray[i] );
				m_bRebuildGOArrow[i] = FALSE;
			}
			if ( m_bRebuildGOPolygon[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOPolygon[i], m_PolygonArray[i] );
				m_bRebuildGOPolygon[i] = FALSE;
			}
			if ( m_bRebuildGOAlignMark[i] )
			{
				CxObjectGenerator::GenerateObjects( m_pGOTable, m_TableGOAlignMark[i], m_AlignMarkArray[i] );
				m_bRebuildGOAlignMark[i] = FALSE;
			}
		
			if ( m_bRebuildGOEllipse[i] )
			{
				GenerateEllipse( i );
				m_bRebuildGOEllipse[i] = FALSE;
			}
		}
	}

	const CxGraphicObjectData& operator = (const CxGraphicObjectData& other)
	{
		for ( int i=0 ; i<MAX_LAYER ; i++ )
		{
			m_nActiveBoxIndex[i]		= other.m_nActiveBoxIndex[i];
			m_nActiveLineIndex[i]		= other.m_nActiveLineIndex[i];
			m_nActiveTextIndex[i]		= other.m_nActiveTextIndex[i];
			m_nActiveCrossIndex[i]		= other.m_nActiveCrossIndex[i];
			m_nActiveDCrossIndex[i]		= other.m_nActiveDCrossIndex[i];
			m_nActiveEllipseIndex[i]	= other.m_nActiveEllipseIndex[i];
			m_nActivePointIndex[i]		= other.m_nActivePointIndex[i];
			m_nActiveArrowIndex[i]		= other.m_nActiveArrowIndex[i];
			m_nActivePolygonIndex[i]	= other.m_nActivePolygonIndex[i];
			m_nActiveAlignMarkIndex[i]	= other.m_nActiveAlignMarkIndex[i];

			m_bRebuildGOBox[i]			= FALSE;
			m_bRebuildGOLine[i]			= FALSE;
			m_bRebuildGOCross[i]		= FALSE;
			m_bRebuildGODCross[i]		= FALSE;
			m_bRebuildGOEllipse[i]		= FALSE;
			m_bRebuildGOPoint[i]		= FALSE;
			m_bRebuildGOArrow[i]		= FALSE;
			m_bRebuildGOPolygon[i]		= FALSE;
			m_bRebuildGOAlignMark[i]	= FALSE;

			m_LayerProperty[i]			= other.m_LayerProperty[i];

			m_BoxArray[i].resize(other.m_BoxArray[i].size());
			if (other.m_BoxArray[i].size() > 0)
				m_bRebuildGOBox[i] = TRUE;
			std::copy(other.m_BoxArray[i].begin(), other.m_BoxArray[i].end(), m_BoxArray[i].begin());
			m_LineArray[i].resize(other.m_LineArray[i].size());
			if (other.m_LineArray[i].size() > 0)
				m_bRebuildGOLine[i] = TRUE;
			std::copy(other.m_LineArray[i].begin(), other.m_LineArray[i].end(), m_LineArray[i].begin());
			m_TextArray[i].resize(other.m_TextArray[i].size());
			std::copy(other.m_TextArray[i].begin(), other.m_TextArray[i].end(), m_TextArray[i].begin());
			m_CrossArray[i].resize(other.m_CrossArray[i].size());
			if (other.m_CrossArray[i].size() > 0)
				m_bRebuildGOCross[i] = TRUE;
			std::copy(other.m_CrossArray[i].begin(), other.m_CrossArray[i].end(), m_CrossArray[i].begin());
			m_DCrossArray[i].resize(other.m_DCrossArray[i].size());
			if (other.m_DCrossArray[i].size() > 0)
				m_bRebuildGODCross[i] = TRUE;
			std::copy(other.m_DCrossArray[i].begin(), other.m_DCrossArray[i].end(), m_DCrossArray[i].begin());
			m_EllipseArray[i].resize(other.m_EllipseArray[i].size());
			if (other.m_EllipseArray[i].size() > 0)
				m_bRebuildGOEllipse[i] = TRUE;
			std::copy(other.m_EllipseArray[i].begin(), other.m_EllipseArray[i].end(), m_EllipseArray[i].begin());
			m_PointArray[i].resize(other.m_PointArray[i].size());
			if (other.m_PointArray[i].size() > 0)
				m_bRebuildGOPoint[i] = TRUE;
			std::copy(other.m_PointArray[i].begin(), other.m_PointArray[i].end(), m_PointArray[i].begin());
			m_ArrowArray[i].resize(other.m_ArrowArray[i].size());
			if (other.m_ArrowArray[i].size() > 0)
				m_bRebuildGOArrow[i] = TRUE;
			std::copy(other.m_ArrowArray[i].begin(), other.m_ArrowArray[i].end(), m_ArrowArray[i].begin());
			m_PolygonArray[i].resize(other.m_PolygonArray[i].size());
			if (other.m_PolygonArray[i].size() > 0)
				m_bRebuildGOPolygon[i] = TRUE;
			std::copy(other.m_PolygonArray[i].begin(), other.m_PolygonArray[i].end(), m_PolygonArray[i].begin());
			m_AlignMarkArray[i].resize(other.m_AlignMarkArray[i].size());
			if (other.m_AlignMarkArray[i].size() > 0)
				m_bRebuildGOAlignMark[i] = TRUE;
			std::copy(other.m_AlignMarkArray[i].begin(), other.m_AlignMarkArray[i].end(), m_AlignMarkArray[i].begin());
		}

		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////
// CxGraphicObjectData::LayerProperty
CxGraphicObjectData::LayerProperty::LayerProperty( BOOL _bVisible, float _fZoomMin, float _fZoomMax ) 
{ bVisible = _bVisible; fZoomMin = _fZoomMin; fZoomMax = _fZoomMax; }
CxGraphicObjectData::LayerProperty::LayerProperty() {}

//////////////////////////////////////////////////////////////////////////
// CxGraphicObject
CxGraphicObject::CxGraphicObject( IxDeviceContext* pIDC/*=NULL*/ ) : 
m_pIDC(pIDC), m_pPointClipper(NULL), m_nViewPortOffset(10), m_hActivePen(NULL), m_bEnableDraw(TRUE)
{
	m_pCsGraphicObject = new CxCriticalSection;
	m_pActiveDefectArrowDrawer = new CxArrowDrawer();
	m_pActiveDefectArrowDrawer->Create( 2, RGB(0xff,0,0), RGB(0xff,0xff,0), RGB(0xff,0,0) );
	
	m_pPointClipper = new CxPointClipper;
		
	m_dwActiveColor	= PDC_GREEN;

	m_pGOTable = CxGOTable::GetRef();

	m_pData = new CxGraphicObjectData(m_pGOTable);

	m_pData->Init();

	wsprintfW( m_wszFontFace, L"Arial" );
}

CxGraphicObject::CxGraphicObject(const CxGraphicObject& other) : 
m_pPointClipper(NULL), m_nViewPortOffset(10), m_hActivePen(NULL), m_bEnableDraw(TRUE)
{
	m_pCsGraphicObject = new CxCriticalSection;
	m_pActiveDefectArrowDrawer = new CxArrowDrawer();
	m_pActiveDefectArrowDrawer->Create( 2, RGB(0xff,0,0), RGB(0xff,0xff,0), RGB(0xff,0,0) );
	
	m_pPointClipper = new CxPointClipper;
		
	m_dwActiveColor	= other.m_dwActiveColor;

	m_pGOTable = CxGOTable::GetRef();

	m_pData = new CxGraphicObjectData(m_pGOTable);

	m_pIDC = other.m_pIDC;
	*m_pData = *other.m_pData;

	wsprintfW( m_wszFontFace, other.m_wszFontFace );
}

const CxGraphicObject& CxGraphicObject::operator = (const CxGraphicObject& other )
{
	m_dwActiveColor	= other.m_dwActiveColor;

	m_pIDC = other.m_pIDC;
	*m_pData = *other.m_pData;

	wsprintfW( m_wszFontFace, other.m_wszFontFace );

	return *this;
}

CxGraphicObject::~CxGraphicObject()
{
	m_pData->DeleteObjects();

	if ( m_pCsGraphicObject )
	{
		delete m_pCsGraphicObject;
		m_pCsGraphicObject = NULL;
	}

	if ( m_hActivePen )
	{
		::DeleteObject( m_hActivePen );
		m_hActivePen = NULL;
	}

	if ( m_pPointClipper )
	{
		delete m_pPointClipper;
		m_pPointClipper = NULL;
	}

	if (m_pActiveDefectArrowDrawer)
	{
		delete m_pActiveDefectArrowDrawer;
		m_pActiveDefectArrowDrawer = NULL;
	}

	delete m_pData;
	//delete m_pGOTable;
	CxGOTable::ReleaseRef();
}

void CxGraphicObject::SetDeviceContext( IxDeviceContext* pIDC )
{
	XASSERT( pIDC );
	m_pIDC = pIDC;
}

void CxGraphicObject::Reset()
{
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
// 	CxGOTable::Instance().Clear();
	m_pData->DeleteObjects();
	m_pData->Clear();
}

void CxGraphicObject::GenerateEllipse( int nLayer )
{
	m_pData->GenerateEllipse(nLayer);
}


void CxGraphicObject::GenerateGraphicObjects()
{
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	
	m_pData->GenerateObjects();
}

BOOL CxGraphicObject::RectIsOverlap( RECT& rcLhs, RECT& rcRhs )
{
	if ( rcLhs.left > rcRhs.left )
	{
		if ( rcLhs.left > rcRhs.right ) return FALSE;
	}
	else
	{
		if ( rcLhs.right < rcRhs.left ) return FALSE;
	}
	
	if ( rcLhs.top > rcRhs.top )
	{
		if ( rcLhs.top > rcRhs.bottom ) return FALSE;
	}
	else
	{
		if ( rcLhs.bottom < rcRhs.top ) return FALSE;
	}
	
	return TRUE;
}

BOOL CxGraphicObject::RectIsContain( RECT& rcLhs, RECT& rcRhs )
{
	return rcLhs.left <= rcRhs.left && rcLhs.right >= rcRhs.right && rcLhs.top <= rcRhs.top && rcLhs.bottom >= rcRhs.bottom;
}

BOOL CxGraphicObject::RectIsOverlapWithLine( RECT& rcLhs, POINT& pt1, POINT& pt2 )
{
	RECT rcBound;
	if ( pt1.x < pt2.x )
	{
		rcBound.left = pt1.x;
		rcBound.right = pt2.x;
	}
	else
	{
		rcBound.left = pt2.x;
		rcBound.right = pt1.x;
	}
	if ( pt1.y < pt2.y )
	{
		rcBound.top = pt1.y;
		rcBound.bottom = pt2.y;
	}
	else
	{
		rcBound.top = pt2.y;
		rcBound.bottom = pt1.y;
	}
	
	if ( rcLhs.left>rcBound.right || rcLhs.right<rcBound.left || rcLhs.top>rcBound.bottom || rcLhs.bottom<rcBound.top )
	{
		return FALSE;
	}
	
	int nDx = pt1.x - pt2.x;
	int nDy = pt1.y - pt2.y;
	
	int nP1 = nDy * ( rcLhs.left - pt2.x ) - nDx * (rcLhs.top - pt2.y );
	int nP2 = nDy * ( rcLhs.left - pt2.x ) - nDx * (rcLhs.bottom - pt2.y );
	int nP3 = nDy * ( rcLhs.right - pt2.x ) - nDx * (rcLhs.top - pt2.y );
	int nP4 = nDy * ( rcLhs.right - pt2.x ) - nDx * (rcLhs.bottom - pt2.y );
	
	if ( nP1 > 0 )
	{
		if ( nP2>0 && nP3>0 && nP4>0 ) return FALSE;
	}
	else if ( nP1 < 0 )
	{
		if ( nP2<0 && nP3<0 && nP4<0 ) return FALSE;
	}
	
	return TRUE;
	
}

void CxGraphicObject::SetLayerVisible( int nLayer, BOOL bVisible/*=TRUE*/ )
{
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return;
	m_pData->m_LayerProperty[nLayer].bVisible = bVisible;
}

void CxGraphicObject::SetAllLayerVisible( BOOL bVisible/*=TRUE*/ )
{
	for ( int i=0 ; i<MAX_LAYER ; i++ )
	{
		m_pData->m_LayerProperty[i].bVisible = bVisible;
	}
}

void CxGraphicObject::SetLayerVisibleRange( int nLayer, float fZoomMin/*=FLT_MIN*/, float fZoomMax/*=FLT_MAX*/ )
{
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return;

	m_pData->m_LayerProperty[nLayer].fZoomMin = fZoomMin;
	m_pData->m_LayerProperty[nLayer].fZoomMax = fZoomMax;
}

BOOL CxGraphicObject::GetLayerVisibleRange( int nLayer, float& fZoomMin, float& fZoomMax )
{
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return FALSE;

	fZoomMin = m_pData->m_LayerProperty[nLayer].fZoomMin;
	fZoomMax = m_pData->m_LayerProperty[nLayer].fZoomMax;

	return TRUE;
}

void CxGraphicObject::ResetLayerVisibleRange()
{
	for ( int i=0 ; i<MAX_LAYER ; i++ )
	{
		m_pData->m_LayerProperty[i].fZoomMin = FLT_MIN;
		m_pData->m_LayerProperty[i].fZoomMax = FLT_MAX;
	}
}

BOOL CxGraphicObject::GetLayerVisible( int nLayer )
{
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return FALSE;

	return m_pData->m_LayerProperty[nLayer].bVisible;
}

void CxGraphicObject::Draw( HDC hDC )
{
	if ( !m_bEnableDraw )
		return;
	
	CxStopWatch StopWatch;
	StopWatch.Begin();
	
	GenerateGraphicObjects();
	
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	
	float fZoomRatio = m_pIDC->GetZoomRatio();

	for ( int i=MAX_LAYER-1 ; i>=0 ; i-- )
	{
		if ( !m_pData->m_LayerProperty[i].bVisible ) continue;

		if ( fZoomRatio < m_pData->m_LayerProperty[i].fZoomMin ) continue;
		if ( fZoomRatio > m_pData->m_LayerProperty[i].fZoomMax ) continue;

		HPEN hOldPen = (HPEN)::SelectObject(hDC, ::GetStockObject(NULL_PEN));
		HBRUSH hOldBrush = (HBRUSH)::SelectObject(hDC, ::GetStockObject(NULL_BRUSH));
		int nOldBkMode = ::SetBkMode( hDC, TRANSPARENT );

		DrawBox( hDC, i );
		DrawCross( hDC, i );
		DrawDCross( hDC, i );
		DrawEllipse( hDC, i );
		DrawLine( hDC, i );
		DrawPolygon( hDC, i );
		DrawAlignMark( hDC, i );
		
		DrawPoint( hDC, i );
		DrawArrow( hDC, i );
		
		DrawText( hDC, i );

		::SetBkMode( hDC, nOldBkMode );
		::SelectObject( hDC, hOldPen );
		::SelectObject( hDC, hOldBrush );
	}
	
	StopWatch.Stop();
	
//	TRACE( _T("CxGraphicObject::Draw - %.2fms Elapsed.\r\n"), StopWatch.GetElapsedTime() );
}

void CxGraphicObject::DrawPolygon( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect(&rcClient, ptScroll.x, ptScroll.y);
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORPOLYGON_ARRAY* pPolygonArray;
	for (CxGraphicObjectData::TableGOPolygon::iterator iter = m_pData->m_TableGOPolygon[nLayer].begin() ; iter != m_pData->m_TableGOPolygon[nLayer].end() ; iter++ )
	{
		hPen = iter->first;
		pPolygonArray = iter->second;

		CxGraphicObjectData::COLORPOLYGON_ARRAY& ArrayPolygon = *pPolygonArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayPolygon.size() ; i++ )
		{
			COLORPOLYGON& clrPolygon = ArrayPolygon.at(i);
			if ( clrPolygon.m_pPenStyle->nThickness == 1 && clrPolygon.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrPolygon.m_pPenStyle->dwBgColor );
			}

			do
			{
				int nPtCnt;
				if ( clrPolygon.bIsFloat )
					nPtCnt = clrPolygon.m_pPolygonDPointArray->Count();
				else
					nPtCnt = clrPolygon.m_pPolygonPointArray->Count();

				if ( nPtCnt > 100 )
				{
					XASSERT( FALSE );
					break;
				}

				DPOINT* lptPolygon = (DPOINT*)alloca( sizeof(DPOINT) * nPtCnt );

				if ( nLayer != 0 )
				{
					if ( clrPolygon.bIsFloat )
					{
						for ( int i=0 ; i<nPtCnt ; i++ )
						{
							DPOINT& pt = clrPolygon.m_pPolygonDPointArray->GetAt(i);
							lptPolygon[i] = m_pIDC->ImagePosToScreenPos( pt.x+.5, pt.y+.5 );
						}
					}
					else
					{
						for ( int i=0 ; i<nPtCnt ; i++ )
						{
							POINT& pt = clrPolygon.m_pPolygonPointArray->GetAt(i);
							lptPolygon[i] = m_pIDC->ImagePosToScreenPos( (double)pt.x, (double)pt.y );
						}
					}
				}
				else
				{
					if ( clrPolygon.bIsFloat )
					{
						for ( int i=0 ; i<nPtCnt ; i++ )
						{
							DPOINT& pt = clrPolygon.m_pPolygonDPointArray->GetAt(i);
							POINT ptOverlay = m_pIDC->ScreenPosToOverlay( int(pt.x+.5), int(pt.y+.5) );
							lptPolygon[i].x = (double)ptOverlay.x;
							lptPolygon[i].y = (double)ptOverlay.y;
						}
					}
					else
					{
						for ( int i=0 ; i<nPtCnt ; i++ )
						{
							POINT& pt = clrPolygon.m_pPolygonPointArray->GetAt(i);
							POINT ptOverlay = m_pIDC->ScreenPosToOverlay( pt.x, pt.y );
							lptPolygon[i].x = (double)ptOverlay.x;
							lptPolygon[i].y = (double)ptOverlay.y;
						}
					}
				}
				
				if ( !::IsOverlapDBoundDPoly( dRectViewPort, nPtCnt, lptPolygon, TRUE ) ) 
					break;

				if ( !m_pPointClipper->ClipPoints( dRectViewPort, lptPolygon, nPtCnt, TRUE ) ) 
					break;
				
				LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
				int nClippedCount = m_pPointClipper->GetClipPointCount();

				if ( nClippedCount < 2 ) break;
				
				if ( m_pData->m_nActivePolygonIndex[nLayer] == clrPolygon.nIndex )
				{
					HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );

					::Polygon( hDC, lpClippedPoints, nClippedCount );

					POINT ptTL1;
					ptTL1.x = (long)(lptPolygon[0].x+0.5);	ptTL1.y = (long)(lptPolygon[0].y+0.5);
					POINT ptTL2;
					ptTL2.x = (long)(lptPolygon[1].x+0.5);	ptTL2.y = (long)(lptPolygon[1].y+0.5);
					ptTL1.x -= 23; ptTL1.y -= 23;
					ptTL2.x -= 3; ptTL2.y -= 3;

					m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_RIGHT );

					::SelectObject( hDC, hOldActivePen );

					break;
				}

				::Polygon( hDC, lpClippedPoints, nClippedCount );

			} while ( FALSE );
			
			if ( clrPolygon.m_pPenStyle->nThickness == 1 && clrPolygon.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawBox( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORBOX_ARRAY* pBoxArray;
	for (CxGraphicObjectData::TableGOBox::iterator iter=m_pData->m_TableGOBox[nLayer].begin() ; iter != m_pData->m_TableGOBox[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pBoxArray = iter->second;

		CxGraphicObjectData::COLORBOX_ARRAY& ArrayBox = *pBoxArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayBox.size() ; i++ )
		{
			COLORBOX& clrBox = ArrayBox.at(i);
			if ( clrBox.m_pPenStyle->nThickness == 1 && clrBox.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrBox.m_pPenStyle->dwBgColor );
			}

			do
			{
				CxDPoint ptdRect[4];
				CxDPoint pt1, pt2;
				
				if ( nLayer != 0 )
				{
					if ( clrBox.bIsFloat )
					{
						pt1 = m_pIDC->ImagePosToScreenPos( clrBox.rcAreaDouble.left+.5,		clrBox.rcAreaDouble.top+.5 );
						pt2 = m_pIDC->ImagePosToScreenPos( clrBox.rcAreaDouble.right+.5,	clrBox.rcAreaDouble.bottom+.5 );
					}
					else
					{
						pt1 = m_pIDC->ImagePosToScreenPos( clrBox.rcAreaInt.left,			clrBox.rcAreaInt.top );
						pt2 = m_pIDC->ImagePosToScreenPos( clrBox.rcAreaInt.right+1,		clrBox.rcAreaInt.bottom+1 );
					}
				}
				else
				{
					if ( clrBox.bIsFloat )
					{
						pt1 = m_pIDC->ScreenPosToOverlay( int(clrBox.rcAreaDouble.left+.5),		int(clrBox.rcAreaDouble.top+.5) );
						pt2 = m_pIDC->ScreenPosToOverlay( int(clrBox.rcAreaDouble.right+.5),	int(clrBox.rcAreaDouble.bottom+.5) );
					}
					else
					{
						pt1 = m_pIDC->ScreenPosToOverlay( clrBox.rcAreaInt.left,			clrBox.rcAreaInt.top );
						pt2 = m_pIDC->ScreenPosToOverlay( clrBox.rcAreaInt.right+1,		clrBox.rcAreaInt.bottom+1 );
					}
				}
				
				CxDRect rcdDevBox( pt1.x, pt1.y, pt2.x, pt2.y );
				
				rcdDevBox.NormalizeRect();
				ptdRect[0].x = rcdDevBox.left;	ptdRect[0].y = rcdDevBox.top;
				ptdRect[1].x = rcdDevBox.right;	ptdRect[1].y = rcdDevBox.top;
				ptdRect[2].x = rcdDevBox.right;	ptdRect[2].y = rcdDevBox.bottom;
				ptdRect[3].x = rcdDevBox.left;	ptdRect[3].y = rcdDevBox.bottom;
				
				if ( !::IsOverlapDBoundDPoly( dRectViewPort, 4, ptdRect, TRUE ) ) 
					break;

				if ( !m_pPointClipper->ClipPoints( dRectViewPort, ptdRect, 4, TRUE ) ) 
					break;
				
				LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
				int nClippedCount = m_pPointClipper->GetClipPointCount();

				if ( nClippedCount < 2 ) break;
				
				if ( m_pData->m_nActiveBoxIndex[nLayer] == clrBox.nIndex )
				{
					HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );

					::Polygon( hDC, lpClippedPoints, nClippedCount );

					RECT rcDevBox = rcdDevBox.ToRECT();
					POINT ptTL1;
					ptTL1.x = rcDevBox.left - 23;
					ptTL1.y = rcDevBox.top - 23;
					POINT ptTL2;
					ptTL2.x = rcDevBox.left - 3;
					ptTL2.y = rcDevBox.top - 3;

					m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_RIGHT );

					::SelectObject( hDC, hOldActivePen );

					break;
				}

				::Polygon( hDC, lpClippedPoints, nClippedCount );

			} while ( FALSE );
			
			if ( clrBox.m_pPenStyle->nThickness == 1 && clrBox.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawEllipse( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;

	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORELLIPSE_ARRAY* pEllipseArray;

	for (CxGraphicObjectData::TableGOEllipse::iterator iter = m_pData->m_TableGOEllipse[nLayer].begin() ; iter != m_pData->m_TableGOEllipse[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pEllipseArray = iter->second;

		CxGraphicObjectData::COLORELLIPSE_ARRAY& ArrayEllipse = *pEllipseArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayEllipse.size() ; i++ )
		{
			COLORELLIPSE& clrEllipse = ArrayEllipse.at(i);
			if ( clrEllipse.m_pPenStyle->nThickness == 1 && clrEllipse.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrEllipse.m_pPenStyle->dwBgColor );
			}

			do
			{
				if ( !clrEllipse.IsPolygon() )
					break;
				
				int nDPtCnt = clrEllipse.m_pPolygonDPointArray->Count();
				DPOINT* pDevPolygon = (DPOINT*)alloca( sizeof(DPOINT)*nDPtCnt );
				for ( int nP = 0 ; nP<nDPtCnt ; nP++ )
				{
					DPOINT& dpP = clrEllipse.m_pPolygonDPointArray->GetAt(nP);
					pDevPolygon[nP] = m_pIDC->ImagePosToScreenPos( dpP.x, dpP.y );
				}
				
				if ( !::IsOverlapDBoundDPoly( dRectViewPort, nDPtCnt, pDevPolygon, TRUE ) ) 
					break;

				if ( !m_pPointClipper->ClipPoints( dRectViewPort, pDevPolygon, nDPtCnt, TRUE ) ) break;
				
				const LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
				int nClippedCount = m_pPointClipper->GetClipPointCount();

				if ( nClippedCount < 2 ) break;
				
				if ( m_pData->m_nActiveEllipseIndex[nLayer] == clrEllipse.nIndex )
				{
					HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );

					::Polygon( hDC, lpClippedPoints, nClippedCount );

					CxDPoint pt1, pt2;
					if ( nLayer != 0 )
					{
						if ( clrEllipse.bIsFloat )
						{
							pt1 = m_pIDC->ImagePosToScreenPos( clrEllipse.rcAreaDouble.left+.5,		clrEllipse.rcAreaDouble.top+.5 );
							pt2 = m_pIDC->ImagePosToScreenPos( clrEllipse.rcAreaDouble.right+.5,	clrEllipse.rcAreaDouble.bottom+.5 );
						}
						else
						{
							pt1 = m_pIDC->ImagePosToScreenPos( clrEllipse.rcAreaInt.left,			clrEllipse.rcAreaInt.top );
							pt2 = m_pIDC->ImagePosToScreenPos( clrEllipse.rcAreaInt.right+1,		clrEllipse.rcAreaInt.bottom+1 );
						}
					}
					else
					{
						if ( clrEllipse.bIsFloat )
						{
							pt1 = m_pIDC->ScreenPosToOverlay( int(clrEllipse.rcAreaDouble.left+.5),		int(clrEllipse.rcAreaDouble.top+.5) );
							pt2 = m_pIDC->ScreenPosToOverlay( int(clrEllipse.rcAreaDouble.right+.5),	int(clrEllipse.rcAreaDouble.bottom+.5) );
						}
						else
						{
							pt1 = m_pIDC->ScreenPosToOverlay( clrEllipse.rcAreaInt.left,			clrEllipse.rcAreaInt.top );
							pt2 = m_pIDC->ScreenPosToOverlay( clrEllipse.rcAreaInt.right+1,		clrEllipse.rcAreaInt.bottom+1 );
						}
					}

					RECT rcDevBox;
					rcDevBox.left = (long)pt1.x;
					rcDevBox.top = (long)pt1.y;
					rcDevBox.right = (long)pt2.x;
					rcDevBox.bottom = (long)pt2.y;
					
					POINT ptTL1, ptTL2;
					ptTL1.x = rcDevBox.left - 23;
					ptTL1.y = rcDevBox.top - 23;
					ptTL2.x = rcDevBox.left - 3;
					ptTL2.y = rcDevBox.top - 3;

					m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_RIGHT );

					::SelectObject( hDC, hOldActivePen );

					break;
				}

				::Polygon( hDC, lpClippedPoints, nClippedCount );
				
			} while ( FALSE );
									
			if ( clrEllipse.m_pPenStyle->nThickness == 1 && clrEllipse.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawCross( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;
	
	HPEN hPen;
	CxGraphicObjectData::COLORCROSS_ARRAY* pCrossArray;
	for (CxGraphicObjectData::TableGOCross::iterator iter=m_pData->m_TableGOCross[nLayer].begin() ; iter != m_pData->m_TableGOCross[nLayer].end() ; iter ++)
	{
		hPen = iter->first;
		pCrossArray = iter->second;

		CxGraphicObjectData::COLORCROSS_ARRAY& ArrayCross = *pCrossArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayCross.size() ; i++ )
		{
			COLORCROSS& clrCross = ArrayCross.at(i);
			if ( clrCross.m_pPenStyle->nThickness == 1 && clrCross.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrCross.m_pPenStyle->dwBgColor );
			}

			do
			{
				CxDPoint dptCross[4];
				if ( nLayer != 0 )
				{
					if ( clrCross.bIsFloat )
					{
						dptCross[0] = m_pIDC->ImagePosToScreenPos( clrCross.ptDouble.x-clrCross.nLength+.5, clrCross.ptDouble.y+.5 );
						dptCross[1] = m_pIDC->ImagePosToScreenPos( clrCross.ptDouble.x+clrCross.nLength+.5, clrCross.ptDouble.y+.5 );
						dptCross[2] = m_pIDC->ImagePosToScreenPos( clrCross.ptDouble.x+.5, clrCross.ptDouble.y-clrCross.nLength+.5 );
						dptCross[3] = m_pIDC->ImagePosToScreenPos( clrCross.ptDouble.x+.5, clrCross.ptDouble.y+clrCross.nLength+.5 );
					}
					else
					{
						dptCross[0] = m_pIDC->ImagePosToScreenPos( clrCross.ptInt.x-clrCross.nLength+.5, clrCross.ptInt.y+.5 );
						dptCross[1] = m_pIDC->ImagePosToScreenPos( clrCross.ptInt.x+clrCross.nLength+.5, clrCross.ptInt.y+.5 );
						dptCross[2] = m_pIDC->ImagePosToScreenPos( clrCross.ptInt.x+.5, clrCross.ptInt.y-clrCross.nLength+.5 );
						dptCross[3] = m_pIDC->ImagePosToScreenPos( clrCross.ptInt.x+.5, clrCross.ptInt.y+clrCross.nLength+.5 );	
					}
				}
				else
				{
					if ( clrCross.bIsFloat )
					{
						dptCross[0] = m_pIDC->ScreenPosToOverlay( int(clrCross.ptDouble.x-clrCross.nLength+.5), int(clrCross.ptDouble.y+.5) );
						dptCross[1] = m_pIDC->ScreenPosToOverlay( int(clrCross.ptDouble.x+clrCross.nLength+.5), int(clrCross.ptDouble.y+.5) );
						dptCross[2] = m_pIDC->ScreenPosToOverlay( int(clrCross.ptDouble.x+.5), int(clrCross.ptDouble.y-clrCross.nLength+.5) );
						dptCross[3] = m_pIDC->ScreenPosToOverlay( int(clrCross.ptDouble.x+.5), int(clrCross.ptDouble.y+clrCross.nLength+.5) );
					}
					else
					{
						dptCross[0] = m_pIDC->ScreenPosToOverlay( clrCross.ptInt.x-clrCross.nLength, clrCross.ptInt.y );
						dptCross[1] = m_pIDC->ScreenPosToOverlay( clrCross.ptInt.x+clrCross.nLength, clrCross.ptInt.y );
						dptCross[2] = m_pIDC->ScreenPosToOverlay( clrCross.ptInt.x, clrCross.ptInt.y-clrCross.nLength );
						dptCross[3] = m_pIDC->ScreenPosToOverlay( clrCross.ptInt.x, clrCross.ptInt.y+clrCross.nLength );	
					}
				}
				
				POINT ptCross[4];
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, dptCross ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptCross, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptCross[0] = lpClippedPoints[0]; ptCross[1] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveCrossIndex[nLayer] == clrCross.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							POINT ptTL1 = dptCross[0].ToPOINT();
							POINT ptTL2 = ptTL1;
							ptTL1.x -= 23; ptTL1.y -= 23;
							ptTL2.x -= 3; ptTL2.y -= 3;

							m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_LEFT );
							
							::MoveToEx( hDC, ptCross[0].x, ptCross[0].y, NULL );
							::LineTo( hDC, ptCross[1].x, ptCross[1].y );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptCross[0].x, ptCross[0].y, NULL );
							::LineTo( hDC, ptCross[1].x, ptCross[1].y );
						}
					}
				}
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, (dptCross+2) ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptCross+2, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptCross[2] = lpClippedPoints[0]; ptCross[3] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveCrossIndex[nLayer] == clrCross.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							
							::MoveToEx( hDC, ptCross[2].x, ptCross[2].y, NULL );
							::LineTo( hDC, ptCross[3].x, ptCross[3].y );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptCross[2].x, ptCross[2].y, NULL );
							::LineTo( hDC, ptCross[3].x, ptCross[3].y );
						}
					}
				}
				
			} while ( FALSE );
				
			if ( clrCross.m_pPenStyle->nThickness == 1 && clrCross.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}

		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawAlignMark( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;


	HPEN hPen;
	CxGraphicObjectData::COLORALIGNMARK_ARRAY* pAlignMarkArray;
	for (CxGraphicObjectData::TableGOAlignMark::iterator iter = m_pData->m_TableGOAlignMark[nLayer].begin() ; iter != m_pData->m_TableGOAlignMark[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pAlignMarkArray = iter->second;

		CxGraphicObjectData::COLORALIGNMARK_ARRAY& ArrayAlignMark = *pAlignMarkArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayAlignMark.size() ; i++ )
		{
			COLORALIGNMARK& clrAlignMark = ArrayAlignMark.at(i);
			if ( clrAlignMark.m_pPenStyle->nThickness == 1 && clrAlignMark.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrAlignMark.m_pPenStyle->dwBgColor );
			}

			do
			{
				CxDPoint dptAlignMark[4];
				CxDPoint dptAlignMarkInnerRect[4];
				if ( nLayer != 0 )
				{
					if ( clrAlignMark.bIsFloat )
					{
						dptAlignMark[0] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x-clrAlignMark.nLength+.5, clrAlignMark.ptDouble.y+.5 );
						dptAlignMark[1] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x+clrAlignMark.nLength+.5, clrAlignMark.ptDouble.y+.5 );
						dptAlignMark[2] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x+.5, clrAlignMark.ptDouble.y-clrAlignMark.nLength+.5 );
						dptAlignMark[3] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x+.5, clrAlignMark.ptDouble.y+clrAlignMark.nLength+.5 );

						dptAlignMarkInnerRect[0] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x-clrAlignMark.nMarkLength+.5, clrAlignMark.ptDouble.y-clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[1] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x+clrAlignMark.nMarkLength+.5, clrAlignMark.ptDouble.y-clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[2] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x+clrAlignMark.nMarkLength+.5, clrAlignMark.ptDouble.y+clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[3] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptDouble.x-clrAlignMark.nMarkLength+.5, clrAlignMark.ptDouble.y+clrAlignMark.nMarkLength+.5 );
					}
					else
					{
						dptAlignMark[0] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x-clrAlignMark.nLength+.5, clrAlignMark.ptInt.y+.5 );
						dptAlignMark[1] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x+clrAlignMark.nLength+.5, clrAlignMark.ptInt.y+.5 );
						dptAlignMark[2] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x+.5, clrAlignMark.ptInt.y-clrAlignMark.nLength+.5 );
						dptAlignMark[3] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x+.5, clrAlignMark.ptInt.y+clrAlignMark.nLength+.5 );	

						dptAlignMarkInnerRect[0] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x-clrAlignMark.nMarkLength+.5, clrAlignMark.ptInt.y-clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[1] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x+clrAlignMark.nMarkLength+.5, clrAlignMark.ptInt.y-clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[2] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x+clrAlignMark.nMarkLength+.5, clrAlignMark.ptInt.y+clrAlignMark.nMarkLength+.5 );
						dptAlignMarkInnerRect[3] = m_pIDC->ImagePosToScreenPos( clrAlignMark.ptInt.x-clrAlignMark.nMarkLength+.5, clrAlignMark.ptInt.y+clrAlignMark.nMarkLength+.5 );	
					}
				}
				else
				{
					if ( clrAlignMark.bIsFloat )
					{
						dptAlignMark[0] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x-clrAlignMark.nLength+.5), int(clrAlignMark.ptDouble.y+.5) );
						dptAlignMark[1] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x+clrAlignMark.nLength+.5), int(clrAlignMark.ptDouble.y+.5) );
						dptAlignMark[2] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x+.5), int(clrAlignMark.ptDouble.y-clrAlignMark.nLength+.5) );
						dptAlignMark[3] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x+.5), int(clrAlignMark.ptDouble.y+clrAlignMark.nLength+.5) );

						dptAlignMarkInnerRect[0] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x-clrAlignMark.nMarkLength+.5), int(clrAlignMark.ptDouble.y-clrAlignMark.nMarkLength+.5) );
						dptAlignMarkInnerRect[1] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x+clrAlignMark.nMarkLength+.5), int(clrAlignMark.ptDouble.y-clrAlignMark.nMarkLength+.5) );
						dptAlignMarkInnerRect[2] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x+clrAlignMark.nMarkLength+.5), int(clrAlignMark.ptDouble.y+clrAlignMark.nMarkLength+.5) );
						dptAlignMarkInnerRect[3] = m_pIDC->ScreenPosToOverlay( int(clrAlignMark.ptDouble.x-clrAlignMark.nMarkLength+.5), int(clrAlignMark.ptDouble.y+clrAlignMark.nMarkLength+.5) );
					}
					else
					{
						dptAlignMark[0] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x-clrAlignMark.nLength, clrAlignMark.ptInt.y );
						dptAlignMark[1] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x+clrAlignMark.nLength, clrAlignMark.ptInt.y );
						dptAlignMark[2] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x, clrAlignMark.ptInt.y-clrAlignMark.nLength );
						dptAlignMark[3] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x, clrAlignMark.ptInt.y+clrAlignMark.nLength );

						dptAlignMarkInnerRect[0] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x-clrAlignMark.nMarkLength, clrAlignMark.ptInt.y-clrAlignMark.nMarkLength );
						dptAlignMarkInnerRect[1] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x+clrAlignMark.nMarkLength, clrAlignMark.ptInt.y-clrAlignMark.nMarkLength );
						dptAlignMarkInnerRect[2] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x+clrAlignMark.nMarkLength, clrAlignMark.ptInt.y+clrAlignMark.nMarkLength );
						dptAlignMarkInnerRect[3] = m_pIDC->ScreenPosToOverlay( clrAlignMark.ptInt.x-clrAlignMark.nMarkLength, clrAlignMark.ptInt.y+clrAlignMark.nMarkLength );
					}
				}

				CxDRect rcdDevBox( dptAlignMarkInnerRect[0].x, dptAlignMarkInnerRect[0].y, dptAlignMarkInnerRect[2].x, dptAlignMarkInnerRect[2].y );
				
				rcdDevBox.NormalizeRect();
				
				POINT ptAlignMark[4];
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, dptAlignMark ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptAlignMark, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptAlignMark[0] = lpClippedPoints[0]; ptAlignMark[1] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveCrossIndex[nLayer] == clrAlignMark.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							POINT ptTL1 = dptAlignMark[0].ToPOINT();
							POINT ptTL2 = ptTL1;
							ptTL1.x -= 23; ptTL1.y -= 23;
							ptTL2.x -= 3; ptTL2.y -= 3;

							m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_LEFT );
							
							::MoveToEx( hDC, ptAlignMark[0].x, ptAlignMark[0].y, NULL );
							::LineTo( hDC, ptAlignMark[1].x, ptAlignMark[1].y );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptAlignMark[0].x, ptAlignMark[0].y, NULL );
							::LineTo( hDC, ptAlignMark[1].x, ptAlignMark[1].y );
						}
					}
				}
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, (dptAlignMark+2) ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptAlignMark+2, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptAlignMark[2] = lpClippedPoints[0]; ptAlignMark[3] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveCrossIndex[nLayer] == clrAlignMark.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							
							::MoveToEx( hDC, ptAlignMark[2].x, ptAlignMark[2].y, NULL );
							::LineTo( hDC, ptAlignMark[3].x, ptAlignMark[3].y );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptAlignMark[2].x, ptAlignMark[2].y, NULL );
							::LineTo( hDC, ptAlignMark[3].x, ptAlignMark[3].y );
						}
					}
				}

				if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptAlignMarkInnerRect, 4, TRUE ) ) 
					break;
				
				LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
				int nClippedCount = m_pPointClipper->GetClipPointCount();

				if ( nClippedCount < 2 ) break;
				
				if ( m_pData->m_nActiveBoxIndex[nLayer] == clrAlignMark.nIndex )
				{
					HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );

					::Polygon( hDC, lpClippedPoints, nClippedCount );

					RECT rcDevBox = rcdDevBox.ToRECT();
					POINT ptTL1;
					ptTL1.x = rcDevBox.left - 23;
					ptTL1.y = rcDevBox.top - 23;
					POINT ptTL2;
					ptTL2.x = rcDevBox.left - 3;
					ptTL2.y = rcDevBox.top - 3;

					m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_RIGHT );

					::SelectObject( hDC, hOldActivePen );

					break;
				}

				::Polygon( hDC, lpClippedPoints, nClippedCount );
				
			} while ( FALSE );
				
			if ( clrAlignMark.m_pPenStyle->nThickness == 1 && clrAlignMark.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}

		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawLine( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORLINE_ARRAY* pLineArray;
	for (CxGraphicObjectData::TableGOLine::iterator iter=m_pData->m_TableGOLine[nLayer].begin() ; iter != m_pData->m_TableGOLine[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pLineArray = iter->second;

		CxGraphicObjectData::COLORLINE_ARRAY& ArrayLine = *pLineArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayLine.size() ; i++ )
		{
			COLORLINE& clrLine = ArrayLine.at(i);
			if ( clrLine.m_pPenStyle->nThickness == 1 && clrLine.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrLine.m_pPenStyle->dwBgColor );
			}
			
			do
			{
				CxDPoint dptLine[2];
				if ( nLayer != 0 )
				{
					if ( clrLine.bIsFloat )
					{
						dptLine[0] = m_pIDC->ImagePosToScreenPos( clrLine.ptDouble[0].x+.5, clrLine.ptDouble[0].y+.5 );
						dptLine[1] = m_pIDC->ImagePosToScreenPos( clrLine.ptDouble[1].x+.5, clrLine.ptDouble[1].y+.5 );
					}
					else
					{
						dptLine[0] = m_pIDC->ImagePosToScreenPos( clrLine.ptInt[0].x+.5, clrLine.ptInt[0].y+.5 );
						dptLine[1] = m_pIDC->ImagePosToScreenPos( clrLine.ptInt[1].x+.5, clrLine.ptInt[1].y+.5 );
					}
				}
				else
				{
					if ( clrLine.bIsFloat )
					{
						dptLine[0] = m_pIDC->ScreenPosToOverlay( int(clrLine.ptDouble[0].x+.5), int(clrLine.ptDouble[0].y+.5) );
						dptLine[1] = m_pIDC->ScreenPosToOverlay( int(clrLine.ptDouble[1].x+.5), int(clrLine.ptDouble[1].y+.5) );
					}
					else
					{
						dptLine[0] = m_pIDC->ScreenPosToOverlay( clrLine.ptInt[0].x, clrLine.ptInt[0].y );
						dptLine[1] = m_pIDC->ScreenPosToOverlay( clrLine.ptInt[1].x, clrLine.ptInt[1].y );
					}
				}

				POINT ptLine[2];
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, dptLine ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptLine, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptLine[0] = lpClippedPoints[0]; ptLine[1] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveLineIndex[nLayer] == clrLine.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							
							::MoveToEx( hDC, ptLine[0].x, ptLine[0].y, NULL );
							::LineTo( hDC, ptLine[1].x, ptLine[1].y );
							
							POINT ptTL1 = dptLine[0].ToPOINT();
							POINT ptTL2 = ptTL1;
							ptTL1.x -= 23; ptTL1.y -= 23;
							ptTL2.x -= 3; ptTL2.y -= 3;
							m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_LEFT );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptLine[0].x, ptLine[0].y, NULL );
							::LineTo( hDC, ptLine[1].x, ptLine[1].y );
						}
					}
				}
				
			} while ( FALSE );
			
			if ( clrLine.m_pPenStyle->nThickness == 1 && clrLine.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawDCross( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;
		
	HPEN hPen;
	CxGraphicObjectData::COLORDCROSS_ARRAY* pDCrossArray;
	for (CxGraphicObjectData::TableGODCross::iterator iter=m_pData->m_TableGODCross[nLayer].begin() ; iter != m_pData->m_TableGODCross[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pDCrossArray = iter->second;

		CxGraphicObjectData::COLORDCROSS_ARRAY& ArrayDCross = *pDCrossArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayDCross.size() ; i++ )
		{
			COLORDCROSS& clrDCross = ArrayDCross.at(i);
			if ( clrDCross.m_pPenStyle->nThickness == 1 && clrDCross.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrDCross.m_pPenStyle->dwBgColor );
			}
			
			do
			{
				CxDPoint dptDCross[4];
				if ( nLayer != 0 )
				{
					if ( clrDCross.bIsFloat )
					{
						dptDCross[0] = m_pIDC->ImagePosToScreenPos( clrDCross.ptDouble.x-clrDCross.nLength+.5, clrDCross.ptDouble.y-clrDCross.nLength+.5 );
						dptDCross[1] = m_pIDC->ImagePosToScreenPos( clrDCross.ptDouble.x+clrDCross.nLength+.5, clrDCross.ptDouble.y+clrDCross.nLength+.5 );
						dptDCross[2] = m_pIDC->ImagePosToScreenPos( clrDCross.ptDouble.x+clrDCross.nLength+.5, clrDCross.ptDouble.y-clrDCross.nLength+.5 );
						dptDCross[3] = m_pIDC->ImagePosToScreenPos( clrDCross.ptDouble.x-clrDCross.nLength+.5, clrDCross.ptDouble.y+clrDCross.nLength+.5 );
					}
					else
					{
						dptDCross[0] = m_pIDC->ImagePosToScreenPos( clrDCross.ptInt.x-clrDCross.nLength+.5, clrDCross.ptInt.y-clrDCross.nLength+.5 );
						dptDCross[1] = m_pIDC->ImagePosToScreenPos( clrDCross.ptInt.x+clrDCross.nLength+.5, clrDCross.ptInt.y+clrDCross.nLength+.5 );
						dptDCross[2] = m_pIDC->ImagePosToScreenPos( clrDCross.ptInt.x+clrDCross.nLength+.5, clrDCross.ptInt.y-clrDCross.nLength+.5 );
						dptDCross[3] = m_pIDC->ImagePosToScreenPos( clrDCross.ptInt.x-clrDCross.nLength+.5, clrDCross.ptInt.y+clrDCross.nLength+.5 );
					}
				}
				else
				{
					if ( clrDCross.bIsFloat )
					{
						dptDCross[0] = m_pIDC->ScreenPosToOverlay( int(clrDCross.ptDouble.x-clrDCross.nLength+.5), int(clrDCross.ptDouble.y-clrDCross.nLength+.5) );
						dptDCross[1] = m_pIDC->ScreenPosToOverlay( int(clrDCross.ptDouble.x+clrDCross.nLength+.5), int(clrDCross.ptDouble.y+clrDCross.nLength+.5) );
						dptDCross[2] = m_pIDC->ScreenPosToOverlay( int(clrDCross.ptDouble.x+clrDCross.nLength+.5), int(clrDCross.ptDouble.y-clrDCross.nLength+.5) );
						dptDCross[3] = m_pIDC->ScreenPosToOverlay( int(clrDCross.ptDouble.x-clrDCross.nLength+.5), int(clrDCross.ptDouble.y+clrDCross.nLength+.5) );
					}
					else
					{
						dptDCross[0] = m_pIDC->ScreenPosToOverlay( clrDCross.ptInt.x-clrDCross.nLength, clrDCross.ptInt.y-clrDCross.nLength );
						dptDCross[1] = m_pIDC->ScreenPosToOverlay( clrDCross.ptInt.x+clrDCross.nLength, clrDCross.ptInt.y+clrDCross.nLength );
						dptDCross[2] = m_pIDC->ScreenPosToOverlay( clrDCross.ptInt.x+clrDCross.nLength, clrDCross.ptInt.y-clrDCross.nLength );
						dptDCross[3] = m_pIDC->ScreenPosToOverlay( clrDCross.ptInt.x-clrDCross.nLength, clrDCross.ptInt.y+clrDCross.nLength );
					}
				}
				
				POINT ptDCross[4];
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, dptDCross ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptDCross, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptDCross[0] = lpClippedPoints[0]; ptDCross[1] = lpClippedPoints[1];
						
						if ( m_pData->m_nActiveDCrossIndex[nLayer] == clrDCross.nIndex )
						{
							HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
							
							::MoveToEx( hDC, ptDCross[0].x, ptDCross[0].y, NULL );
							::LineTo( hDC, ptDCross[1].x, ptDCross[1].y );
							
							POINT ptTL1 = dptDCross[0].ToPOINT();
							POINT ptTL2 = ptTL1;
							ptTL1.x -= 23; ptTL1.y -= 23;
							ptTL2.x -= 3; ptTL2.y -= 3;
							m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_LEFT );
							
							::SelectObject( hDC, hOldActivePen );
						}
						else
						{
							::MoveToEx( hDC, ptDCross[0].x, ptDCross[0].y, NULL );
							::LineTo( hDC, ptDCross[1].x, ptDCross[1].y );
						}
					}
				}
				
				if ( ::IsOverlapDBoundDLine( dRectViewPort, (dptDCross+2) ) )
				{
					if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptDCross+2, 2, FALSE ) ) break;
					LPPOINT lpClippedPoints = m_pPointClipper->GetClipPoints();
					if ( m_pPointClipper->GetClipPointCount() == 2 )
					{
						ptDCross[2] = lpClippedPoints[0]; ptDCross[3] = lpClippedPoints[1];
						
						::MoveToEx( hDC, ptDCross[2].x, ptDCross[2].y, NULL );
						::LineTo( hDC, ptDCross[3].x, ptDCross[3].y );
					}
				}

			} while ( FALSE );
			
			if ( clrDCross.m_pPenStyle->nThickness == 1 && clrDCross.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
	}
}

void CxGraphicObject::DrawPoint( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORPOINT_ARRAY* pPointArray;
	for (CxGraphicObjectData::TableGOPoint::iterator iter=m_pData->m_TableGOPoint[nLayer].begin() ; iter != m_pData->m_TableGOPoint[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pPointArray = iter->second;

		CxGraphicObjectData::COLORPOINT_ARRAY& ArrayPoint = *pPointArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );
		HGDIOBJ hOldBrush = NULL;

		if ( (int)ArrayPoint.size() != 0 )
		{
			COLORPOINT& clrPoint = ArrayPoint.at(0);
			if ( clrPoint.m_hBrush )
			{
				::SelectObject( hDC, clrPoint.m_hBrush );
			}
		}
		
		for ( int i=0 ; i<(int)ArrayPoint.size() ; i++ )
		{
			COLORPOINT& clrPoint = ArrayPoint.at(i);
			
			POINT ptPoint[2];
			if ( nLayer != 0 )
			{
				ptPoint[0] = m_pIDC->ImagePosToScreenPos( clrPoint.ptInt.x, clrPoint.ptInt.y );
				ptPoint[1] = m_pIDC->ImagePosToScreenPos( clrPoint.ptInt.x+1, clrPoint.ptInt.y+1 );
			}
			else
			{
				ptPoint[0] = m_pIDC->ScreenPosToOverlay( clrPoint.ptInt.x, clrPoint.ptInt.y );
				ptPoint[1] = m_pIDC->ScreenPosToOverlay( clrPoint.ptInt.x+1, clrPoint.ptInt.y+1 );
			}
			
			RECT rcDevBox;
			rcDevBox.left = ptPoint[0].x;
			rcDevBox.top = ptPoint[0].y;
			rcDevBox.right = ptPoint[1].x;
			rcDevBox.bottom = ptPoint[1].y;
			
			if ( !RectIsOverlap( rcClient, rcDevBox ) ) continue;
			
			::NormalizeRect( &rcDevBox );
			
			if ( m_pData->m_nActiveBoxIndex[nLayer] == clrPoint.nIndex )
			{
				HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
				
				POINT ptTL1;
				ptTL1.x = rcDevBox.left - 23;
				ptTL1.y = rcDevBox.top - 23;
				POINT ptTL2;
				ptTL2.x = rcDevBox.left - 3;
				ptTL2.y = rcDevBox.top - 3;

				m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_RIGHT );
				
				if ( (rcDevBox.right-rcDevBox.left) <= 0 || (rcDevBox.bottom-rcDevBox.top) <= 0 )
					::SetPixel( hDC, rcDevBox.left, rcDevBox.top, m_dwActiveColor );
				else
					::Rectangle( hDC, rcDevBox.left, rcDevBox.top, rcDevBox.right, rcDevBox.bottom );
				
				::SelectObject( hDC, hOldActivePen );
			}
			else
			{
				if ( (rcDevBox.right-rcDevBox.left) <= 0 || (rcDevBox.bottom-rcDevBox.top) <= 0 )
					::SetPixel( hDC, rcDevBox.left, rcDevBox.top, clrPoint.m_pPenStyle->dwFgColor );
				else
					::Rectangle( hDC, rcDevBox.left, rcDevBox.top, rcDevBox.right, rcDevBox.bottom );
			}
		}
		
		::SelectObject( hDC, hOldPen );
		if ( hOldBrush )
			::SelectObject( hDC, hOldBrush );
	}
}

void CxGraphicObject::DrawArrowHead( HDC hDC, CxGOArrow& Arrow, POINT pt[2], BOOL bStartClip, BOOL bEndClip,
									double dSlopeY1, double dCosY1, double dSinY1, 
									double dSlopeY2, double dCosY2, double dSinY2, double dPar )
{
	if ( !bEndClip && (Arrow.direction & COLORARROW::ArrowDirectionEnd) )
	{
		int nTX1 = int(dPar * dCosY1 - ( dPar / 2.0 * dSinY1 ) );//+ .5f);
		int nTX2 = int(dPar * dCosY1 + ( dPar / 2.0 * dSinY1 ) );//+ .5f);
		int nTY1 = int(dPar * dSinY1 + ( dPar / 2.0 * dCosY1 ) );//+ .5f);
		int nTY2 = int(dPar * dSinY1 - ( dPar / 2.0 * dCosY1 ) );//+ .5f);
		
		if ( Arrow.headType == COLORARROW::ArrowHeadTypeTriangle )
		{
			// Draw the arrow head
			::BeginPath(hDC);
			
			::MoveToEx( hDC, pt[1].x, pt[1].y, NULL );
			::LineTo( hDC, pt[1].x + nTX1, pt[1].y + nTY1 );
			::LineTo( hDC, pt[1].x + nTX2, pt[1].y + nTY2 );
			::LineTo( hDC, pt[1].x, pt[1].y );
			
			::EndPath(hDC);
			::StrokeAndFillPath(hDC);
		}
		else
		{
			::MoveToEx( hDC, pt[1].x, pt[1].y, NULL );
			::LineTo( hDC, pt[1].x + nTX1, pt[1].y + nTY1 );
			::MoveToEx( hDC, pt[1].x, pt[1].y, NULL );
			::LineTo( hDC, pt[1].x + nTX2, pt[1].y + nTY2 );
		}
	}
	
	if ( !bStartClip && (Arrow.direction & COLORARROW::ArrowDirectionStart) )
	{
		int nTX1 = int(dPar * dCosY2 - ( dPar / 2.0 * dSinY2 ) );//+ .5f);
		int nTX2 = int(dPar * dCosY2 + ( dPar / 2.0 * dSinY2 ) );//+ .5f);
		int nTY1 = int(dPar * dSinY2 + ( dPar / 2.0 * dCosY2 ) );//+ .5f);
		int nTY2 = int(dPar * dSinY2 - ( dPar / 2.0 * dCosY2 ) );//+ .5f);
		
		if ( Arrow.headType == COLORARROW::ArrowHeadTypeTriangle )
		{
			::BeginPath(hDC);
			
			::MoveToEx( hDC, pt[0].x, pt[0].y, NULL );
			::LineTo( hDC, pt[0].x + nTX1, pt[0].y + nTY1 );
			::LineTo( hDC, pt[0].x + nTX2, pt[0].y + nTY2 );
			::LineTo( hDC, pt[0].x, pt[0].y );	
			
			::EndPath(hDC);
			::StrokeAndFillPath(hDC);
		}
		else
		{
			::MoveToEx( hDC, pt[0].x, pt[0].y, NULL );
			::LineTo( hDC, pt[0].x + nTX1, pt[0].y + nTY1 );
			::MoveToEx( hDC, pt[0].x, pt[0].y, NULL );
			::LineTo( hDC, pt[0].x + nTX2, pt[0].y + nTY2 );	
		}
	}
}

void CxGraphicObject::DrawArrow( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	HPEN hPen;
	CxGraphicObjectData::COLORARROW_ARRAY* pArrowArray;
	for (CxGraphicObjectData::TableGOArrow::iterator iter=m_pData->m_TableGOArrow[nLayer].begin() ; iter != m_pData->m_TableGOArrow[nLayer].end() ; iter++)
	{
		hPen = iter->first;
		pArrowArray = iter->second;

		CxGraphicObjectData::COLORARROW_ARRAY& ArrayArrow = *pArrowArray;
		
		HGDIOBJ hOldPen = ::SelectObject( hDC, hPen );

		HGDIOBJ hOldBrush = NULL;
		
		if ( (int)ArrayArrow.size() != 0 )
		{
			COLORARROW& clrArrow = ArrayArrow.at(0);
			if ( clrArrow.m_hBrush )
			{
				::SelectObject( hDC, clrArrow.m_hBrush );
			}
		}
		
		int nOldBkMode, nOldBkColor;
		for ( int i=0 ; i<(int)ArrayArrow.size() ; i++ )
		{
			COLORARROW& clrArrow = ArrayArrow.at(i);
			if ( clrArrow.m_pPenStyle->nThickness == 1 && clrArrow.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				nOldBkMode = ::SetBkMode( hDC, OPAQUE );
				nOldBkColor = ::SetBkColor( hDC, clrArrow.m_pPenStyle->dwBgColor );
			}
			
			do
			{
				CxDPoint dptArrow[2];
				if ( nLayer != 0 )
				{
					if ( clrArrow.bIsFloat )
					{
						dptArrow[0] = m_pIDC->ImagePosToScreenPos( clrArrow.ptDouble[0].x+.5, clrArrow.ptDouble[0].y+.5 );
						dptArrow[1] = m_pIDC->ImagePosToScreenPos( clrArrow.ptDouble[1].x+.5, clrArrow.ptDouble[1].y+.5 );
					}
					else
					{
						dptArrow[0] = m_pIDC->ImagePosToScreenPos( clrArrow.ptInt[0].x+.5, clrArrow.ptInt[0].y+.5 );
						dptArrow[1] = m_pIDC->ImagePosToScreenPos( clrArrow.ptInt[1].x+.5, clrArrow.ptInt[1].y+.5 );
					}
				}
				else
				{
					if ( clrArrow.bIsFloat )
					{
						dptArrow[0] = m_pIDC->ScreenPosToOverlay( int(clrArrow.ptDouble[0].x+.5), int(clrArrow.ptDouble[0].y+.5) );
						dptArrow[1] = m_pIDC->ScreenPosToOverlay( int(clrArrow.ptDouble[1].x+.5), int(clrArrow.ptDouble[1].y+.5) );
					}
					else
					{
						dptArrow[0] = m_pIDC->ScreenPosToOverlay( clrArrow.ptInt[0].x, clrArrow.ptInt[0].y );
						dptArrow[1] = m_pIDC->ScreenPosToOverlay( clrArrow.ptInt[1].x, clrArrow.ptInt[1].y );
					}
				}
				
				POINT ptArrow[2];
				
				if ( !::IsOverlapDBoundDLine( dRectViewPort, dptArrow ) ) break;

				if ( !m_pPointClipper->ClipPoints( dRectViewPort, dptArrow, 2, FALSE ) ) break;

				LPDPOINT lpClippedPoints = m_pPointClipper->GetClipDPoints();

				if ( m_pPointClipper->GetClipPointCount() == 2 )
				{
					BOOL bStartClip, bEndClip;
					bStartClip =	(dptArrow[0] == lpClippedPoints[0]) ? FALSE : TRUE;
					bEndClip =		(dptArrow[1] == lpClippedPoints[1]) ? FALSE : TRUE;

					ptArrow[0].x = (long)(lpClippedPoints[0].x+0.5); ptArrow[0].y = (long)(lpClippedPoints[0].y+0.5);
					ptArrow[1].x = (long)(lpClippedPoints[1].x+0.5); ptArrow[1].y = (long)(lpClippedPoints[1].y+0.5);

					int nArrowLen = (ptArrow[0].x-ptArrow[1].x)*(ptArrow[0].x-ptArrow[1].x) + (ptArrow[0].y-ptArrow[1].y)*(ptArrow[0].y-ptArrow[1].y);
					if ( nArrowLen <= clrArrow.size*clrArrow.size ) break;

					double dSlopeY1 , dCosY1 , dSinY1;
					double dSlopeY2 , dCosY2 , dSinY2;
					double dPar = clrArrow.size;		// arrow head GetSize
					
					dSlopeY1 = atan2( double(ptArrow[0].y - ptArrow[1].y), double(ptArrow[0].x - ptArrow[1].x) );
					dCosY1 = cos( dSlopeY1 );
					dSinY1 = sin( dSlopeY1 );
					
					dSlopeY2 = atan2( double(ptArrow[1].y - ptArrow[0].y), double(ptArrow[1].x - ptArrow[0].x) );
					dCosY2 = cos( dSlopeY2 );
					dSinY2 = sin( dSlopeY2 );
					
					if ( m_pData->m_nActiveArrowIndex[nLayer] == clrArrow.nIndex )
					{
						HGDIOBJ hOldActivePen = ::SelectObject( hDC, m_hActivePen );
						
						::MoveToEx( hDC, ptArrow[0].x, ptArrow[0].y, NULL );
						::LineTo( hDC, ptArrow[1].x, ptArrow[1].y );

						DrawArrowHead(	hDC, clrArrow, ptArrow, bStartClip, bEndClip, 
										dSlopeY1, dCosY1, dSinY1, 
										dSlopeY2, dCosY2, dSinY2, dPar );
						
						POINT ptTL1 = dptArrow[0].ToPOINT();
						POINT ptTL2 = ptTL1;
						ptTL1.x -= 23; ptTL1.y -= 23;
						ptTL2.x -= 3; ptTL2.y -= 3;
						m_pActiveDefectArrowDrawer->Draw( hDC, ptTL1, ptTL2, CxArrowDrawer::AT_LEFT );
						
						::SelectObject( hDC, hOldActivePen );
					}
					else
					{
						::MoveToEx( hDC, ptArrow[0].x, ptArrow[0].y, NULL );
						::LineTo( hDC, ptArrow[1].x, ptArrow[1].y );

						DrawArrowHead(	hDC, clrArrow, ptArrow, bStartClip, bEndClip, 
										dSlopeY1, dCosY1, dSinY1, 
										dSlopeY2, dCosY2, dSinY2, dPar );
					}
				}
				
			} while ( FALSE );
			
			if ( clrArrow.m_pPenStyle->nThickness == 1 && clrArrow.m_pPenStyle->dwBgColor != PDC_NULL )
			{
				::SetBkMode( hDC, nOldBkMode );
				::SetBkColor( hDC, nOldBkColor );
			}
		}
		
		::SelectObject( hDC, hOldPen );
		if ( hOldBrush )
			::SelectObject( hDC, hOldBrush );
	}
}

void CxGraphicObject::SetFontFace( LPCTSTR lpszFaceName )
{
	USES_CONVERSION;
	wcscpy_s( m_wszFontFace, T2W((LPTSTR)lpszFaceName) );
}

void CxGraphicObject::DrawText( HDC hDC, int nLayer )
{
	XASSERT( m_pIDC );
	if (!m_pIDC)
		return;

	RECT rcClient;
	m_pIDC->GetClientRect(&rcClient);
	POINT ptScroll = m_pIDC->GetDeviceScrollPosition();
	::OffsetRect( &rcClient, ptScroll.x, ptScroll.y );
	
	rcClient.left -= m_nViewPortOffset;
	rcClient.top -= m_nViewPortOffset;
	rcClient.right += m_nViewPortOffset;
	rcClient.bottom += m_nViewPortOffset;
	
	CxDRect dRectViewPort;
	dRectViewPort = rcClient;

	RECT rcBlock;
	int nOldBkMode = ::SetBkMode( hDC, TRANSPARENT );
	DWORD dwOldTextColor = ::SetTextColor( hDC, RGB(0xff, 0, 0) );

	int nOldTextHeight = 0;
	HFONT hTextFont = NULL;

	HFONT hOldFont = (HFONT)::GetCurrentObject( hDC, OBJ_FONT );
	for ( UINT i=0 ; i<(UINT)m_pData->m_TextArray[nLayer].size() ; i++ )
	{
		COLORTEXT& Text = m_pData->m_TextArray[nLayer].at(i);

		float fZoomRatio = m_pIDC->GetZoomRatio();

		int nTextHeight = 0;

		int lfHeight = -MulDiv(Text.nHeight, GetDeviceCaps(hDC, LOGPIXELSY), 72);

		if ( Text.bDynamic && (fZoomRatio < 1.f) )
			nTextHeight = int(lfHeight * fZoomRatio + .5f);
		else
			nTextHeight = lfHeight;

		if ( abs(nTextHeight) <= 3 )
			//nTextHeight = -3;
			continue;

		if ( nOldTextHeight != nTextHeight )
		{
			if ( hTextFont ) ::DeleteObject( hTextFont );
			LOGFONT lf;
			memset( &lf, 0, sizeof(LOGFONT) );
			USES_CONVERSION;
			wsprintf( lf.lfFaceName, W2T(m_wszFontFace) );
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lf.lfQuality = DEFAULT_QUALITY;
			lf.lfPitchAndFamily = FF_DONTCARE;	
			lf.lfWeight = FW_BOLD;
			lf.lfHeight = nTextHeight;
			hTextFont = ::CreateFontIndirect( &lf );
			::SelectObject( hDC, hTextFont );
			nOldTextHeight = nTextHeight;
		}

		SIZE szText;
		::GetTextExtentPoint32W( hDC, Text.wszText, 
			lstrlenW(Text.wszText), &szText ); 

		POINT pt1;
		if (nLayer != 0)
		{
			DPOINT dpt = m_pIDC->ImagePosToScreenPos( Text.pt.x+.5, Text.pt.y+.5 );
			pt1.x = (long)(dpt.x+0.5);	pt1.y = (long)(dpt.y+0.5);
		}
		else
		{
			pt1 = m_pIDC->ScreenPosToOverlay( Text.pt.x, Text.pt.y );
		}

		switch ( Text.eTextAlignment )
		{
		case CxGOText::TextAlignmentCenter:
			rcBlock.left	= pt1.x - szText.cx/2;
			rcBlock.right	= pt1.x + szText.cx/2;
			rcBlock.top		= pt1.y - szText.cy/2;
			rcBlock.bottom	= pt1.y + szText.cy/2;
			break;
		case CxGOText::TextAlignmentLeft:
			rcBlock.left	= pt1.x - szText.cx;
			rcBlock.right	= pt1.x;
			rcBlock.top		= pt1.y - szText.cy/2;
			rcBlock.bottom	= pt1.y + szText.cy/2;
			break;
		case CxGOText::TextAlignmentRight:
			rcBlock.left	= pt1.x;
			rcBlock.right	= pt1.x + szText.cx;
			rcBlock.top		= pt1.y - szText.cy/2;
			rcBlock.bottom	= pt1.y + szText.cy/2;
			break;
		}

		if ( !RectIsOverlap( rcClient, rcBlock ) ) continue;

		if ( Text.dwBgColor == PDC_NULL )
			::SetTextColor( hDC, RGB(0xff,0xff,0xff) );
		else
			::SetTextColor( hDC, Text.dwBgColor );
		rcBlock.left--; rcBlock.right--;
		::DrawTextW( hDC, Text.wszText, lstrlenW(Text.wszText), &rcBlock, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
		rcBlock.left+=2; rcBlock.right+=2;
		::DrawTextW( hDC, Text.wszText, lstrlenW(Text.wszText), &rcBlock, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
		rcBlock.left--; rcBlock.right--;
		rcBlock.top--; rcBlock.bottom--;
		::DrawTextW( hDC, Text.wszText, lstrlenW(Text.wszText), &rcBlock, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
		rcBlock.top+=2; rcBlock.bottom+=2;
		::DrawTextW( hDC, Text.wszText, lstrlenW(Text.wszText), &rcBlock, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
		rcBlock.top--; rcBlock.bottom--;

		::SetTextColor( hDC, Text.dwFgColor );
		::DrawTextW( hDC, Text.wszText, lstrlenW(Text.wszText), &rcBlock, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOCLIP );
	}

	::SelectObject( hDC, hOldFont );
	::DeleteObject( hTextFont );

	::SetBkMode( hDC, nOldBkMode );
	::SetTextColor( hDC, dwOldTextColor );
}

void CxGraphicObject::SetActiveGOColor( COLORREF ActiveColor )
{
	if ( m_dwActiveColor != ActiveColor )
	{
		if ( m_hActivePen ) ::DeleteObject( m_hActivePen );
		m_hActivePen = ::CreatePen( PS_SOLID, 1, ActiveColor );
		m_dwActiveColor = ActiveColor;
	}
}

int CxGraphicObject::AddDrawObject( CxGOBasic* pGOBasic, int nLayer/* = MAX_LAYER-1*/ )
{
	if ( !m_bEnableDraw ) return -1;
	if ( !pGOBasic ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	
	int nIndex = -1;
	switch ( pGOBasic->m_GraphicObjectType )
	{
	case CxGOBasic::GraphicObjectTypeBox:
		nIndex = AddDrawBox( *((CxGOBox*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeArrow:
		nIndex = AddDrawArrow( *((CxGOArrow*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeCross:
		nIndex = AddDrawCross( *((CxGOCross*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeDiagonalCross:
		nIndex = AddDrawDCross( *((CxGODCross*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeEllipse:
		nIndex = AddDrawEllipse( *((CxGOEllipse*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeLine:
		nIndex = AddDrawLine( *((CxGOLine*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypePoint:
		nIndex = AddDrawPoint( *((CxGOPoint*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeText:
		nIndex = AddDrawText( *((CxGOText*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypePolygon:
		nIndex = AddDrawPolygon( *((CxGOPolygon*)(pGOBasic)), nLayer );
		break;
	case CxGOBasic::GraphicObjectTypeAlignMark:
		nIndex = AddDrawAlignMark( *((CxGOAlignMark*)(pGOBasic)), nLayer );
		break;
	default:
		XASSERT(FALSE);
		break;
	}
	
	return nIndex;
}

int CxGraphicObject::AddDrawBox( CxGOBox& Box, int nLayer/* = MAX_LAYER-1*/ ) 
{ 
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOBox[nLayer] = TRUE;
	Box.nIndex = (int)m_pData->m_BoxArray[nLayer].size();
	Box.NormalizeRect();
	m_pData->m_BoxArray[nLayer].push_back( Box ); return (int)m_pData->m_BoxArray[nLayer].size()-1; 
}
int CxGraphicObject::AddDrawEllipse( CxGOEllipse& Ellipse, int nLayer/* = MAX_LAYER-1*/ )
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOEllipse[nLayer] = TRUE;
	Ellipse.nIndex = (int)m_pData->m_EllipseArray[nLayer].size();
	Ellipse.NormalizeRect();
	m_pData->m_EllipseArray[nLayer].push_back( Ellipse ); return (int)m_pData->m_EllipseArray[nLayer].size()-1;
}
int CxGraphicObject::AddDrawLine( CxGOLine& Line, int nLayer/* = MAX_LAYER-1*/ ) 
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOLine[nLayer] = TRUE;
	Line.nIndex = (int)m_pData->m_LineArray[nLayer].size();
	m_pData->m_LineArray[nLayer].push_back( Line ); return (int)m_pData->m_LineArray[nLayer].size()-1; 
}
int CxGraphicObject::AddDrawArrow( CxGOArrow& Arrow, int nLayer/* = MAX_LAYER-1*/ )
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOArrow[nLayer] = TRUE;
	Arrow.nIndex = (int)m_pData->m_ArrowArray[nLayer].size();
	m_pData->m_ArrowArray[nLayer].push_back( Arrow ); return (int)m_pData->m_ArrowArray[nLayer].size()-1;
}
int CxGraphicObject::AddDrawText( CxGOText& Text, int nLayer/* = MAX_LAYER-1*/ ) 
{ 
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	Text.nIndex = (int)m_pData->m_TextArray[nLayer].size();
	m_pData->m_TextArray[nLayer].push_back( Text ); return (int)m_pData->m_TextArray[nLayer].size()-1; 
}
int CxGraphicObject::AddDrawCross( CxGOCross& Cross, int nLayer/* = MAX_LAYER-1*/ ) 
{ 
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOCross[nLayer] = TRUE;
	Cross.nIndex = (int)m_pData->m_CrossArray[nLayer].size();
	m_pData->m_CrossArray[nLayer].push_back( Cross ); return (int)m_pData->m_CrossArray[nLayer].size()-1; 
}
int CxGraphicObject::AddDrawDCross( CxGODCross& DCross, int nLayer/* = MAX_LAYER-1*/ ) 
{ 
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGODCross[nLayer] = TRUE;
	DCross.nIndex = (int)m_pData->m_DCrossArray[nLayer].size();
	m_pData->m_DCrossArray[nLayer].push_back( DCross ); return (int)m_pData->m_DCrossArray[nLayer].size()-1; 
}
int CxGraphicObject::AddDrawPoint( CxGOPoint& Point, int nLayer/* = MAX_LAYER-1*/ )
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOPoint[nLayer] = TRUE;
	Point.nIndex = (int)m_pData->m_PointArray[nLayer].size();
	m_pData->m_PointArray[nLayer].push_back( Point ); return (int)m_pData->m_PointArray[nLayer].size()-1;
}
int CxGraphicObject::AddDrawPolygon( CxGOPolygon& GPolygon, int nLayer /*=MAX_LAYER-1*/ )
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOPolygon[nLayer] = TRUE;
	GPolygon.nIndex = (int)m_pData->m_PolygonArray[nLayer].size();
	m_pData->m_PolygonArray[nLayer].push_back( GPolygon ); return (int)m_pData->m_PolygonArray[nLayer].size()-1;
}
int CxGraphicObject::AddDrawAlignMark( CxGOAlignMark& AlignMark, int nLayer /* = MAX_LAYER-1  */)
{
	if ( !m_bEnableDraw ) return -1;
	if ( nLayer >= MAX_LAYER || nLayer < 0 ) return -1;
	CxCriticalSection::Owner Lock(*m_pCsGraphicObject);
	m_pData->m_bRebuildGOAlignMark[nLayer] = TRUE;
	AlignMark.nIndex = (int)m_pData->m_AlignMarkArray[nLayer].size();
	m_pData->m_AlignMarkArray[nLayer].push_back( AlignMark ); return (int)m_pData->m_AlignMarkArray[nLayer].size()-1;
}
void CxGraphicObject::EnableDraw( BOOL bEnable )
{
	m_bEnableDraw = bEnable;
}

void CxGraphicObject::SetOffsetViewPort( int nOffset ) { m_nViewPortOffset = nOffset; }
int CxGraphicObject::GetLayerCount() { return MAX_LAYER; }

void CxGraphicObject::SetActiveTextIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ ) 
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveTextIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveCrossIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ )  
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveCrossIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveDCrossIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ ) 
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveDCrossIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveEllipseIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ )
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveEllipseIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveArrowIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ )
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveArrowIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveBoxIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ ) 
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveBoxIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveLineIndex( int nIndex, int nLayer/* = MAX_LAYER-1*/ ) 
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveLineIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActivePolygonIndex( int nIndex, int nLayer /* = MAX_LAYER-1*/ )
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActivePolygonIndex[nLayer] = nIndex; }
void CxGraphicObject::SetActiveAlignMarkIndex( int nIndex, int nLayer /* = MAX_LAYER-1*/ )
{ if ( nLayer >= MAX_LAYER || nLayer < 0 ) return; m_pData->m_nActiveAlignMarkIndex[nLayer] = nIndex; }
