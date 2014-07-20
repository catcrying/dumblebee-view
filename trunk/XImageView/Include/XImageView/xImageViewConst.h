#ifndef __X_IMAGE_VIEW_CONST_H__
#define __X_IMAGE_VIEW_CONST_H__

namespace ImageViewEvent
{
	enum Event { 
		ButtonEventZoomInClick		= 0x101,
		ButtonEventZoomOutClick		= 0x102,
		ButtonEventZoomInOutClick	= 0x103,
		ButtonEventZoomNotClick		= 0x104,
		ButtonEventZoomFitClick		= 0x105,
		ButtonEventLoadClick		= 0x106,
		ButtonEventSaveClick		= 0x107,
		ButtonEventMaximizeClick	= 0x110,
		ButtonEventMeasureClick		= 0x111,

		ActionEventPanStart			= 0x200,
		ActionEventPanEnd			= 0x201,
		ActionEventZoomIn			= 0x202,
		ActionEventZoomOut			= 0x203,
		ActionEventLoadFile			= 0x206,
		ActionEventSaveFile			= 0x207,
		ActionEventMaximizedScreen	= 0x208,
		ActionEventNormalScreen		= 0x209,
		ActionEventTitleLButtonDown	= 0x20A,
		ActionEventTitleLButtonUp	= 0x20B
	};
}

namespace ImageViewMode
{
	enum ScreenMode { 
		ScreenModeNone, 
		ScreenModePanning, 
		ScreenModeZoomIn, 
		ScreenModeZoomOut, 
		ScreenModeZoomInOut, 
		ScreenModeMeasure, 
		ScreenModeTracker 
	};
	enum MouseWheelMode { 
		MouseWheelModeZoom, 
		MouseWheelModeVerticalScroll,
		MouseWheelModeHorizontalScroll
	};
}

class IxDeviceContext;
typedef void (APIENTRY *FnOnDrawExt)( IxDeviceContext* pIDC, CDC* pDC, UINT nIndexData, LPVOID lpUsrData );
typedef void (APIENTRY *FnOnMeasure)( float fValueX, float fValueY, UINT nIndexData, LPVOID lpUsrData );
typedef void (APIENTRY *FnOnConfirmTracker)( CRect& rcTrackRegion, UINT nIndexData, LPVOID lpUsrData );

typedef BOOL (APIENTRY* FnOnFireMouseEvent) ( DWORD dwMsg, CPoint& point, UINT nIndexData, LPVOID lpUsrData );
typedef BOOL (APIENTRY* FnOnEvent) ( ImageViewEvent::Event evt, UINT nIndexData, LPVOID lpUsrData );

struct REGISTER_CALLBACK
{
	FnOnDrawExt				fnOnDrawExt;
	FnOnMeasure				fnOnMeasure;
	FnOnConfirmTracker		fnOnConfirmTracker;

	FnOnFireMouseEvent		fnOnFireMouseEvent;
	FnOnEvent				fnOnEvent;

	LPVOID					lpUsrData[5];
};

#endif //__X_IMAGE_VIEW_CONST_H__