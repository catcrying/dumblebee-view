#pragma once

#include <Gdiplus.h>

using namespace Gdiplus;
GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadius );
GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadius );
GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB );
GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB );
Image* GdipLoadImageFromRes( HMODULE hResHandle, LPCTSTR lpszResType, UINT nId );