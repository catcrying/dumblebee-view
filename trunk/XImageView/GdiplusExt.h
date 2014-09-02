/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#pragma once

#include <Gdiplus.h>

using namespace Gdiplus;
GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadius );
GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadius );
GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB );
GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB );
Image* GdipLoadImageFromRes( HMODULE hResHandle, LPCTSTR lpszResType, UINT nId );