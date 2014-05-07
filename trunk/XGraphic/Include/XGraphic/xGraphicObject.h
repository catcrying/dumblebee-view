// xGraphicObject.h: interface for the CxGraphicObject class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAPHICOBJECT_H__C1B2DDF3_C33F_4A88_85FE_262B2287F2A4__INCLUDED_)
#define AFX_GRAPHICOBJECT_H__C1B2DDF3_C33F_4A88_85FE_262B2287F2A4__INCLUDED_

#include <XGraphic/export.h>
#include <XGraphic/IxDeviceContext.h>
#include <XGraphic/xDataTypes.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_LAYER				(20)

// pre-defined color list (147)
// standard: SVG
#define PDC_ALICEBLUE				RGB(240,248,255)
#define PDC_ANTIQUEWHITE			RGB(250,235,215)
#define PDC_AQUA					RGB(0,255,255)
#define PDC_AQUAMARINE				RGB(127,255,212)
#define PDC_AZURE					RGB(240,255,255)
#define PDC_BEIGE					RGB(245,245,220)
#define PDC_BISQUE					RGB(255,228,196)
#define PDC_BLACK					RGB(0,0,0)
#define PDC_BLANCHEDALMOND			RGB(255,235,205)
#define PDC_BLUE					RGB(0,0,255)
#define PDC_BLUEVIOLET				RGB(138,43,226)
#define PDC_BROWN					RGB(165,42,42)
#define PDC_BURLYWOOD				RGB(222,184,135)
#define PDC_CADETBLUE				RGB(95,158,160)
#define PDC_CHARTREUSE				RGB(127,255,0)
#define PDC_CHOCOLATE				RGB(210,105,30)
#define PDC_CORAL					RGB(255,127,80)
#define PDC_CORNFLOWERBLUE			RGB(100,149,237)
#define PDC_CORNSILK				RGB(255,248,220)
#define PDC_CRIMSON					RGB(220,20,60)
#define PDC_CYAN					RGB(0,255,255)
#define PDC_DARKBLUE				RGB(0,0,139)
#define PDC_DARKCYAN				RGB(0,139,139)
#define PDC_DARKGOLDENROD			RGB(184,134,11)
#define PDC_DARKGRAY				RGB(169,169,169)
#define PDC_DARKGREEN				RGB(0,100,0)
#define PDC_DARKGREY				RGB(169,169,169)
#define PDC_DARKKHAKI				RGB(189,183,107)
#define PDC_DARKMAGENTA				RGB(139,0,139)
#define PDC_DARKOLIVEGREEN			RGB(85,107,47)
#define PDC_DARKORANGE				RGB(255,140,0)
#define PDC_DARKORCHID				RGB(153,50,204)
#define PDC_DARKRED					RGB(139,0,0)
#define PDC_DARKSALMON				RGB(233,150,122)
#define PDC_DARKSEAGREEN			RGB(143,188,143)
#define PDC_DARKSLATEBLUE			RGB(72,61,139)
#define PDC_DARKSLATEGRAY			RGB(47,79,79)
#define PDC_DARKSLATEGREY			RGB(47,79,79)
#define PDC_DARKTURQUOISE			RGB(0,206,209)
#define PDC_DARKVIOLET				RGB(148,0,211)
#define PDC_DEEPPINK				RGB(255,20,147)
#define PDC_DEEPSKYBLUE				RGB(0,191,255)
#define PDC_DIMGRAY					RGB(105,105,105)
#define PDC_DIMGREY					RGB(105,105,105)
#define PDC_DODGERBLUE				RGB(30,144,255)
#define PDC_FIREBRICK				RGB(178,34,34)
#define PDC_FLORALWHITE				RGB(255,250,240)
#define PDC_FORESTGREEN				RGB(34,139,34)
#define PDC_FUCHSIA					RGB(255,0,255)
#define PDC_GAINSBORO				RGB(220,220,220)
#define PDC_GHOSTWHITE				RGB(248,248,255)
#define PDC_GOLD					RGB(255,215,0)
#define PDC_GOLDENROD				RGB(218,165,32)
#define PDC_GRAY					RGB(128,128,128)
#define PDC_GREEN					RGB(0,128,0)
#define PDC_GREENYELLOW				RGB(173,255,47)
#define PDC_GREY					RGB(128,128,128)
#define PDC_HONEYDEW				RGB(240,255,240)
#define PDC_HOTPINK					RGB(255,105,180)
#define PDC_INDIANRED				RGB(205,92,92)
#define PDC_INDIGO					RGB(75,0,130)
#define PDC_IVORY					RGB(255,255,240)
#define PDC_KHAKI					RGB(240,230,140)
#define PDC_LAVENDER				RGB(230,230,250)
#define PDC_LAVENDERBLUSH			RGB(255,240,245)
#define PDC_LAWNGREEN				RGB(124,252,0)
#define PDC_LEMONCHIFFON			RGB(255,250,205)
#define PDC_LIGHTBLUE				RGB(173,216,230)
#define PDC_LIGHTCORAL				RGB(240,128,128)
#define PDC_LIGHTCYAN				RGB(224,255,255)
#define PDC_LIGHTGOLDENRODYELLOW	RGB(250,250,210)
#define PDC_LIGHTGRAY				RGB(211,211,211)
#define PDC_LIGHTGREEN				RGB(144,238,144)
#define PDC_LIGHTGREY				RGB(211,211,211)
#define PDC_LIGHTPINK				RGB(255,182,193)
#define PDC_LIGHTSALMON				RGB(255,160,122)
#define PDC_LIGHTSEAGREEN			RGB(32,178,170)
#define PDC_LIGHTSKYBLUE			RGB(135,206,250)
#define PDC_LIGHTSLATEGRAY			RGB(119,136,153)
#define PDC_LIGHTSLATEGREY			RGB(119,136,153)
#define PDC_LIGHTSTEELBLUE			RGB(176,196,222)
#define PDC_LIGHTYELLOW				RGB(255,255,224)
#define PDC_LIME					RGB(0,255,0)
#define PDC_LIMEGREEN				RGB(50,205,50)
#define PDC_LINEN					RGB(250,240,230)
#define PDC_MAGENTA					RGB(255,0,255)
#define PDC_MAROON					RGB(128,0,0)
#define PDC_MEDIUMAQUAMARINE		RGB(102,205,170)
#define PDC_MEDIUMBLUE				RGB(0,0,205)
#define PDC_MEDIUMORCHID			RGB(186,85,211)
#define PDC_MEDIUMPURPLE			RGB(147,112,219)
#define PDC_MEDIUMSEAGREEN			RGB(60,179,113)
#define PDC_MEDIUMSLATEBLUE			RGB(123,104,238)
#define PDC_MEDIUMSPRINGGREEN		RGB(0,250,154)
#define PDC_MEDIUMTURQUOISE			RGB(72,209,204)
#define PDC_MEDIUMVIOLETRED			RGB(199,21,133)
#define PDC_MIDNIGHTBLUE			RGB(25,25,112)
#define PDC_MINTCREAM				RGB(245,255,250)
#define PDC_MISTYROSE				RGB(255,228,225)
#define PDC_MOCCASIN				RGB(255,228,181)
#define PDC_NAVAJOWHITE				RGB(255,222,173)
#define PDC_NAVY					RGB(0,0,128)
#define PDC_OLDLACE					RGB(253,245,230)
#define PDC_OLIVE					RGB(128,128,0)
#define PDC_OLIVEDRAB				RGB(107,142,35)
#define PDC_ORANGE					RGB(255,165,0)
#define PDC_ORANGERED				RGB(255,69,0)
#define PDC_ORCHID					RGB(218,112,214)
#define PDC_PALEGOLDENROD			RGB(238,232,170)
#define PDC_PALEGREEN				RGB(152,251,152)
#define PDC_PALETURQUOISE			RGB(175,238,238)
#define PDC_PALEVIOLETRED			RGB(219,112,147)
#define PDC_PAPAYAWHIP				RGB(255,239,213)
#define PDC_PEACHPUFF				RGB(255,218,185)
#define PDC_PERU					RGB(205,133,63)
#define PDC_PINK					RGB(255,192,203)
#define PDC_PLUM					RGB(221,160,221)
#define PDC_POWDERBLUE				RGB(176,224,230)
#define PDC_PURPLE					RGB(128,0,128)
#define PDC_RED						RGB(255,0,0)
#define PDC_ROSYBROWN				RGB(188,143,143)
#define PDC_ROYALBLUE				RGB(65,105,225)
#define PDC_SADDLEBROWN				RGB(139,69,19)
#define PDC_SALMON					RGB(250,128,114)
#define PDC_SANDYBROWN				RGB(244,164,96)
#define PDC_SEAGREEN				RGB(46,139,87)
#define PDC_SEASHELL				RGB(255,245,238)
#define PDC_SIENNA					RGB(160,82,45)
#define PDC_SILVER					RGB(192,192,192)
#define PDC_SKYBLUE					RGB(135,206,235)
#define PDC_SLATEBLUE				RGB(106,90,205)
#define PDC_SLATEGRAY				RGB(112,128,144)
#define PDC_SLATEGREY				RGB(112,128,144)
#define PDC_SNOW					RGB(255,250,250)
#define PDC_SPRINGGREEN				RGB(0,255,127)
#define PDC_STEELBLUE				RGB(70,130,180)
#define PDC_TAN						RGB(210,180,140)
#define PDC_TEAL					RGB(0,128,128)
#define PDC_THISTLE					RGB(216,191,216)
#define PDC_TOMATO					RGB(255,99,71)
#define PDC_TURQUOISE				RGB(64,224,208)
#define PDC_VIOLET					RGB(238,130,238)
#define PDC_WHEAT					RGB(245,222,179)
#define PDC_WHITE					RGB(255,255,255)
#define PDC_WHITESMOKE				RGB(245,245,245)
#define PDC_YELLOW					RGB(255,255,0)
#define PDC_YELLOWGREEN				RGB(154,205,50)
#define PDC_NULL					(DWORD)-1

// non-standard (4)
#define PDC_LIGHTRED				PDC_RED
#define PDC_LIGHTMAGENTA			PDC_FUCHSIA
#define PDC_DEEPRED					RGB(225,0,87)
#define PDC_DEEPBLUE				RGB(92,2,251)

class CxGraphicObject;
class CxObjectGenerator;
class CxPenStyle;
class CxBrushStyle;
class XGRAPHIC_API CxGOBasic
{
friend class CxGraphicObject;
friend class CxObjectGenerator;
public:
	enum GraphicObjectType {
		GraphicObjectTypeNone, 
		GraphicObjectTypeBox, 
		GraphicObjectTypeArrow, 
		GraphicObjectTypeCross, 
		GraphicObjectTypeDiagonalCross, 
		GraphicObjectTypeEllipse, 
		GraphicObjectTypeLine, 
		GraphicObjectTypePoint, 
		GraphicObjectTypeText,
		GraphicObjectTypePolygon,
		GraphicObjectTypeAlignMark
	};	
protected:
	HPEN			m_hPen;
	HBRUSH			m_hBrush;
	CxPenStyle*		m_pPenStyle;
	CxBrushStyle*	m_pBrushStyle;
	GraphicObjectType	m_GraphicObjectType;

public:
	const CxGOBasic& operator = ( const CxGOBasic& other );
	CxGOBasic( const CxGOBasic& other );

	GraphicObjectType GetType();
	CxGOBasic();
	virtual ~CxGOBasic();
};

class XGRAPHIC_API CxGOBox : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	DRECT		rcAreaDouble;
	RECT		rcAreaInt;
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:
	
	void CreateObject( COLORREF dwFgColor, int nLeft, int nTop, int nRight, int nBottom, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double dLeft, double dTop, double dRight, double dBottom, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, RECT rcArea, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DRECT rcArea, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void NormalizeRect();

	CxGOBox();
	virtual ~CxGOBox();
};

#define COLORBOX CxGOBox

class PolygonDPointArray;
class XGRAPHIC_API CxGOEllipse : public CxGOBasic
{
friend class CxGraphicObject;
friend class CxGraphicObjectData;
protected:
	DRECT		rcAreaDouble;
	RECT		rcAreaInt;
	int			nIndex;
	PolygonDPointArray*	m_pPolygonDPointArray;
	BOOL		bIsFloat;
	int			nSplitDiv;

	void ToPolygon();
	BOOL IsPolygon();
public:

	void SetSplitDiv( int nDiv );
	void CreateObject( COLORREF dwFgColor, int nLeft, int nTop, int nRight, int nBottom, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double dLeft, double dTop, double dRight, double dBottom, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, RECT rcArea, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DRECT rcArea, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void NormalizeRect();

	const CxGOEllipse& operator = ( const CxGOEllipse& Other );
	CxGOEllipse( const CxGOEllipse& Other );

	CxGOEllipse();
	virtual ~CxGOEllipse();
};

#define COLORELLIPSE CxGOEllipse

class XGRAPHIC_API CxGOLine : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	POINT		ptInt[2];
	DPOINT		ptDouble[2];
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:

	void CreateObject( COLORREF dwFgColor, int x1, int y1, int x2, int y2, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double x1, double y1, double x2, double y2, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, POINT pt1, POINT pt2, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DPOINT pt1, DPOINT pt2, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );

	CxGOLine();
	virtual ~CxGOLine();
};

#define COLORLINE CxGOLine

class XGRAPHIC_API CxGOArrow : public CxGOBasic
{
friend class CxGraphicObject;
public:
	enum ArrowDirection { ArrowDirectionStart=0x01, ArrowDirectionEnd=0x02, ArrowDirectionBoth=0x03 };
	enum ArrowHeadType { ArrowHeadTypeLine=0x00, ArrowHeadTypeTriangle=0x03 };

protected:
	POINT		ptInt[2];
	DPOINT		ptDouble[2];
	ArrowDirection	direction;
	ArrowHeadType	headType;
	int			size;
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:

	void CreateObject( COLORREF dwFgColor, int x1, int y1, int x2, int y2, 
		ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle=PS_SOLID );
	void CreateObject( COLORREF dwFgColor, double x1, double y1, double x2, double y2, 
		ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle=PS_SOLID );
	void CreateObject( COLORREF dwFgColor, POINT pt1, POINT pt2, 
		ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle=PS_SOLID );
	void CreateObject( COLORREF dwFgColor, DPOINT pt1, DPOINT pt2, 
		ArrowDirection arrowDirection, ArrowHeadType arrowType, int nArrowSize, int nLineStyle=PS_SOLID );

	CxGOArrow();
	virtual ~CxGOArrow();
};

#define COLORARROW CxGOArrow

class XGRAPHIC_API CxGODCross : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	POINT		ptInt;
	DPOINT		ptDouble;
	int			nLength;
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:

	void CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DPOINT pt, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	CxGODCross();
	virtual ~CxGODCross();
};

#define COLORALIGNMARK	CxGOAlignMark

class XGRAPHIC_API CxGOAlignMark : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	POINT		ptInt;
	DPOINT		ptDouble;
	int			nLength;
	int			nMarkLength;
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:

	void CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nMarkLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nMarkLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nMarkLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DPOINT pt, int nLength, int nMarkLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );

	CxGOAlignMark();
	virtual ~CxGOAlignMark();
};

#define COLORDCROSS CxGODCross


class XGRAPHIC_API CxGOCross : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	POINT		ptInt;
	DPOINT		ptDouble;
	int			nLength;
	int			nIndex;		// assign by CxImageScrollView
	BOOL		bIsFloat;
public:

	void CreateObject( COLORREF dwFgColor, int x, int y, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, double x, double y, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, POINT pt, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, CxDPoint pt, int nLength, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );

	CxGOCross();
	virtual ~CxGOCross();
};

#define COLORCROSS CxGOCross

class XGRAPHIC_API CxGOPoint : public CxGOBasic
{
friend class CxGraphicObject;
protected:
	POINT		ptInt;
	int			nIndex;		// assign by CxImageScrollView
public:

	void CreateObject( COLORREF dwFgColor, int x, int y );
	void CreateObject( COLORREF dwFgColor, POINT pt );
	CxGOPoint();
	virtual ~CxGOPoint();
};

#define COLORPOINT CxGOPoint

class XGRAPHIC_API CxGOText : public CxGOBasic
{
friend class CxGraphicObject;
public:
	enum TextAlignment { TextAlignmentLeft, TextAlignmentRight, TextAlignmentCenter };

protected:
	POINT		pt;
	int			nHeight;
	WCHAR		wszText[256];
	int			nIndex;		// assign by CxImageScrollView
	TextAlignment eTextAlignment;
	BOOL		bDynamic;
	COLORREF	dwFgColor;
	COLORREF	dwBgColor;
public:
	void CreateObject( COLORREF _dwFgColor, int _x, int _y, int _nHeight, BOOL _bDynamic=FALSE, TextAlignment _eStyle=TextAlignmentRight, COLORREF _dwBgColor = -1 );
	void CreateObject( COLORREF _dwFgColor, POINT pt, int _nHeight, BOOL _bDynamic=FALSE, TextAlignment _eStyle=TextAlignmentRight, COLORREF _dwBgColor = -1 );
	void SetText( LPCTSTR lpszFormat, ... );

	CxGOText();
	virtual ~CxGOText();
};

#define COLORTEXT CxGOText

class PolygonPointArray;
class PolygonDPointArray;
class XGRAPHIC_API CxGOPolygon : public CxGOBasic
{
friend class CxGraphicObject;
public:

protected:
	int			nIndex;		// assign by CxImageScrollView
	PolygonPointArray*	m_pPolygonPointArray;
	PolygonDPointArray*	m_pPolygonDPointArray;
	BOOL		bClosed;
	COLORREF	dwFgColor;
	COLORREF	dwBgColor;
	BOOL		bIsFloat;

public:
	void CreateObject( COLORREF dwFgColor, POINT* lptPolygon, int nPtCnt, BOOL bClosed = TRUE, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );
	void CreateObject( COLORREF dwFgColor, DPOINT* lptPolygon, int nPtCnt, BOOL bClosed = TRUE, int nStyle = PS_SOLID, int nThickness=1, COLORREF dwBgColor=-1 );

	const CxGOPolygon& operator = ( const CxGOPolygon& Other );
	CxGOPolygon( const CxGOPolygon& Other );

	CxGOPolygon();
	virtual ~CxGOPolygon();
};

#define COLORPOLYGON CxGOPolygon

#ifndef FLT_MIN
#define FLT_MIN         1.175494351e-38F        /* min positive value */
#endif

#ifndef FLT_MAX
#define FLT_MAX         3.402823466e+38F        /* max value */
#endif

class CxPointClipper;
class IxDeviceContext;
class CxCriticalSection;
class CxArrowDrawer;
class CxGraphicObjectData;
class CxGOTable;
class XGRAPHIC_API CxGraphicObject  
{
protected:
	CxGraphicObjectData*	m_pData;
	BOOL					m_bEnableDraw;

	CxCriticalSection*		m_pCsGraphicObject;

	CxArrowDrawer*			m_pActiveDefectArrowDrawer;

	HPEN					m_hActivePen;

	CxGOTable*				m_pGOTable;

	IxDeviceContext* m_pIDC;

	COLORREF		m_dwActiveColor;

	CxPointClipper*	m_pPointClipper;

	int				m_nViewPortOffset;

	WCHAR			m_wszFontFace[256];

	void DrawBox( HDC hDC, int nLayer );
	void DrawLine( HDC hDC, int nLayer );
	void DrawText( HDC hDC, int nLayer );
	void DrawCross( HDC hDC, int nLayer );
	void DrawDCross( HDC hDC, int nLayer );
	void DrawEllipse( HDC hDC, int nLayer );
	void DrawPoint( HDC hDC, int nLayer );
	void DrawArrow( HDC hDC, int nLayer );
	void DrawPolygon( HDC hDC, int nLayer );
	void DrawAlignMark( HDC hDC, int nLayer );

	void DrawArrowHead( HDC hDC, CxGOArrow& Arrow, POINT pt[2], BOOL bStartClip, BOOL bEndClip,
						double dSlopeY1, double dCosY1, double dSinY1, 
						double dSlopeY2, double dCosY2, double dSinY2, double dPar );


	void GenerateGraphicObjects();

	void GenerateEllipse( int nLayer );

	BOOL RectIsOverlapWithLine( RECT& rcLhs, POINT& pt1, POINT& pt2 );
	BOOL RectIsContain( RECT& rcLhs, RECT& rcRhs );
	BOOL RectIsOverlap( RECT& rcLhs, RECT& rcRhs );
	
public:
	CxGraphicObject(const CxGraphicObject& other);
	const CxGraphicObject& operator = (const CxGraphicObject& other );
	CxGraphicObject( IxDeviceContext* pIDC=NULL );
	~CxGraphicObject();

	void SetDeviceContext( IxDeviceContext* pIDC );

	//////////////////////////////////////////////////////////////////////////
	// graphic object
	void SetActiveBoxIndex		( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveLineIndex		( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveTextIndex		( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveCrossIndex	( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveDCrossIndex	( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveEllipseIndex	( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveArrowIndex	( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActivePolygonIndex	( int nIndex, int nLayer = MAX_LAYER-1 );
	void SetActiveAlignMarkIndex( int nIndex, int nLayer = MAX_LAYER-1 );

	void SetActiveGOColor( COLORREF ActiveColor );

	void SetFontFace( LPCTSTR lpszFaceName );

	void SetOffsetViewPort( int nOffset );

	int AddDrawObject( CxGOBasic* pGOBasic, int nLayer = MAX_LAYER-1 );

	int AddDrawBox( COLORBOX& Box, int nLayer = MAX_LAYER-1 );
	int AddDrawEllipse( COLORELLIPSE& Ellipse, int nLayer = MAX_LAYER-1 );
	int AddDrawLine( COLORLINE& Line, int nLayer = MAX_LAYER-1 );
	int AddDrawArrow( COLORARROW& Arrow, int nLayer = MAX_LAYER-1 );
	int AddDrawText( COLORTEXT& Text, int nLayer = MAX_LAYER-1 );
	int AddDrawCross( COLORCROSS& Cross, int nLayer = MAX_LAYER-1 );
	int AddDrawDCross( COLORDCROSS& DCross, int nLayer = MAX_LAYER-1 );
	int AddDrawPoint( COLORPOINT& Point, int nLayer = MAX_LAYER-1 );
	int AddDrawPolygon( COLORPOLYGON& GPolygon, int nLayer = MAX_LAYER-1 );
	int AddDrawAlignMark( COLORALIGNMARK& AlignMark, int nLayer = MAX_LAYER-1 );

	void Reset();

	void EnableDraw( BOOL bEnable );

	void SetLayerVisible( int nLayer, BOOL bVisible=TRUE );
	BOOL GetLayerVisible( int nLayer );

	void SetLayerVisibleRange( int nLayer, float fZoomMin=FLT_MIN, float fZoomMax=FLT_MAX );
	BOOL GetLayerVisibleRange( int nLayer, float& fZoomMin, float& fZoomMax );

	void SetAllLayerVisible( BOOL bVisible = TRUE );
	void ResetLayerVisibleRange();

	int GetLayerCount();

	//////////////////////////////////////////////////////////////////////////
	// draw
	void Draw( HDC hDC );
};


#endif // !defined(AFX_GRAPHICOBJECT_H__C1B2DDF3_C33F_4A88_85FE_262B2287F2A4__INCLUDED_)
