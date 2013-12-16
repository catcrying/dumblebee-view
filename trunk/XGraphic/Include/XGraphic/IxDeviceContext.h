#ifndef __I_DEVICE_CONTEXT_H__
#define __I_DEVICE_CONTEXT_H__

#include <XGraphic/xDataTypes.h>

class IxDeviceContext
{
public:

	//////////////////////////////////////////////////////////////////////////
	// coordinate conversion
	virtual POINT MousePosToImagePos( int nMouseX, int nMouseY, BOOL* pbIsValidPos=NULL ) = 0;
	virtual RECT MousePosToImagePos( int nMouseLeft, int nMouseTop, int nMouseRight, int nMouseBottom ) = 0;
	virtual DPOINT ImagePosToScreenPos( double dX, double dY ) = 0;
	virtual POINT ImagePosToScreenPos( int x, int y ) = 0;
	virtual RECT ImagePosToScreenPos( int nLeft, int nTop, int nRight, int nBottom ) = 0;
	virtual DRECT ImagePosToScreenPos( double dLeft, double dTop, double dRight, double dBottom ) = 0;
	virtual POINT ScreenPosToOverlay( int x, int y ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Retreieve information
	virtual int GetViewWidth() = 0;
	virtual int GetViewHeight() = 0;
	virtual void GetCurrentViewingCoordinate( int& nX1, int& nY1, int& nX2, int& nY2 ) = 0;
	virtual void GetClientRect( LPRECT lpRect ) = 0;
	virtual POINT GetDeviceScrollPosition() const = 0;
	virtual float GetZoomRatio() = 0;

	IxDeviceContext() {}
	virtual ~IxDeviceContext() {}
};


#endif //__I_DEVICE_CONTEXT_H__