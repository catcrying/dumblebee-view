#include "StdAfx.h"
#include "GdiplusExt.h"
#include <atlbase.h>

#pragma comment( lib, "GdiPlus.lib" )

GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadius )
{
	GraphicsPath roundRect;
	int nDiameter = nRadius << 1;
	Rect arcRect( 0, 0, nDiameter, nDiameter );

	arcRect.X = rect.GetLeft();
	arcRect.Y = rect.GetTop();
	roundRect.AddArc(arcRect, 180, 90);
	arcRect.X = rect.GetRight() - nDiameter;
	roundRect.AddArc(arcRect, 270, 90);
	arcRect.Y = rect.GetBottom() - nDiameter;
	roundRect.AddArc(arcRect, 0, 90);
	arcRect.X = rect.GetLeft();
	roundRect.AddArc(arcRect, 90, 90);
	roundRect.CloseFigure();

	return roundRect.Clone();
}

GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadius )
{
	GraphicsPath roundRect;
	float fDiameter = nRadius * 2.f;
	RectF arcRect( 0.f, 0.f, fDiameter, fDiameter );

	arcRect.X = rect.GetLeft();
	arcRect.Y = rect.GetTop();
	roundRect.AddArc(arcRect, 180, 90);
	arcRect.X = rect.GetRight() - fDiameter;
	roundRect.AddArc(arcRect, 270, 90);
	arcRect.Y = rect.GetBottom() - fDiameter;
	roundRect.AddArc(arcRect, 0, 90);
	arcRect.X = rect.GetLeft();
	roundRect.AddArc(arcRect, 90, 90);
	roundRect.CloseFigure();

	return roundRect.Clone();
}

GraphicsPath* GdipCreateRoundRect( Rect& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB )
{
	GraphicsPath roundRect;
	if( nRadiusLT == 0 )
		roundRect.AddLine( rect.GetLeft(), rect.GetTop(), rect.GetRight() - (nRadiusRT<<1), rect.GetTop() );
	else
		roundRect.AddArc( Rect(rect.GetLeft(), rect.GetTop(), nRadiusLT<<1, nRadiusLT<<1), 180, 90 );

	if( nRadiusRT == 0 )
		roundRect.AddLine( rect.GetRight(), rect.GetTop(), rect.GetRight(), rect.GetBottom()-(nRadiusRB<<1) );
	else
		roundRect.AddArc( Rect(rect.GetRight() - (nRadiusRT<<1), rect.GetTop(), nRadiusRT<<1, nRadiusRT<<1), 270, 90 );

	if( nRadiusRB == 0 )
		roundRect.AddLine( rect.GetRight(), rect.GetBottom(), rect.GetLeft()-(nRadiusLB<<1), rect.GetBottom() );
	else
		roundRect.AddArc( Rect(rect.GetRight() - (nRadiusRB<<1), rect.GetBottom() -(nRadiusRB<<1), nRadiusRB<<1, nRadiusRB<<1), 0, 90 );

	if( nRadiusLB == 0 )
		roundRect.AddLine( rect.GetLeft(), rect.GetBottom(), rect.GetLeft(), rect.GetTop()-(nRadiusLT<<1) );
	else
		roundRect.AddArc( Rect(rect.GetLeft(), rect.GetBottom() -(nRadiusLB<<1), nRadiusLB<<1, nRadiusLB<<1), 90, 90 );

	roundRect.CloseFigure();

	return roundRect.Clone();
}

GraphicsPath* GdipCreateRoundRect( RectF& rect, int nRadiusLT, int nRadiusRT, int nRadiusRB, int nRadiusLB )
{
	GraphicsPath roundRect;
	if( nRadiusLT == 0 )
		roundRect.AddLine( rect.GetLeft(), rect.GetTop(), rect.GetRight() - (nRadiusRT<<1), rect.GetTop() );
	else
		roundRect.AddArc( RectF(rect.GetLeft(), rect.GetTop(), (float)(nRadiusLT<<1), (float)(nRadiusLT<<1)), 180, 90 );

	if( nRadiusRT == 0 )
		roundRect.AddLine( rect.GetRight(), rect.GetTop(), rect.GetRight(), rect.GetBottom()-(float)(nRadiusRB<<1) );
	else
		roundRect.AddArc( RectF(rect.GetRight() - (float)(nRadiusRT<<1), rect.GetTop(), (float)(nRadiusRT<<1), (float)(nRadiusRT<<1)), 270, 90 );

	if( nRadiusRB == 0 )
		roundRect.AddLine( rect.GetRight(), rect.GetBottom(), rect.GetLeft()-(float)(nRadiusLB<<1), rect.GetBottom() );
	else
		roundRect.AddArc( RectF(rect.GetRight() - (float)(nRadiusRB<<1), rect.GetBottom() -(float)(nRadiusRB<<1), (float)(nRadiusRB<<1), (float)(nRadiusRB<<1)), 0, 90 );

	if( nRadiusLB == 0 )
		roundRect.AddLine( rect.GetLeft(), rect.GetBottom(), rect.GetLeft(), rect.GetTop()-(float)(nRadiusLT<<1) );
	else
		roundRect.AddArc( RectF(rect.GetLeft(), rect.GetBottom() -(float)(nRadiusLB<<1), (float)(nRadiusLB<<1), (float)(nRadiusLB<<1)), 90, 90 );

	roundRect.CloseFigure();

	return roundRect.Clone();
}

Image* GdipLoadImageFromRes( HMODULE hResHandle, LPCTSTR lpszResType, UINT nId )
{
	HRSRC hResource = ::FindResource( hResHandle, MAKEINTRESOURCE(nId), lpszResType );
	if( !hResource ) return NULL;

	DWORD dwImageSize = ::SizeofResource( hResHandle, hResource );
	if( !dwImageSize ) return NULL;

	const void* pResData = ::LockResource( ::LoadResource( hResHandle, hResource ) );
	if( !pResData ) return NULL;

	HGLOBAL hBuffer = ::GlobalAlloc( GMEM_MOVEABLE, dwImageSize );
	if( !hBuffer ) return NULL;

	void* pBuffer = ::GlobalLock( hBuffer );
	if( !pBuffer )
	{
		::GlobalFree( hBuffer );
		return NULL;
	}

	::CopyMemory( pBuffer, pResData, dwImageSize );

	CComPtr<IStream> spStream = NULL;
	if( ::CreateStreamOnHGlobal( hBuffer, FALSE, &spStream ) != S_OK )
	{
		::GlobalUnlock( hBuffer );
		::GlobalFree( hBuffer );
		return NULL;
	}

	Image* pRetImage = Image::FromStream( spStream );

	::GlobalUnlock( hBuffer );
	::GlobalFree( hBuffer );

	return pRetImage;
}