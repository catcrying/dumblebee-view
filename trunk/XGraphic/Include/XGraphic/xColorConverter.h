#pragma once

#include <XGraphic/export.h>
#include <wtypes.h>

namespace xColorConverter
{

void XGRAPHIC_API ConvertRGBToHSL( COLORREF rgb, float *H, float *S, float *L );
COLORREF XGRAPHIC_API GetRGBFromHLSExtend( float H, float L, float S );
COLORREF XGRAPHIC_API GetRGBFromHLS( float H, float L, float S );
BYTE XGRAPHIC_API GetRGBFromHue( float rm1, float rm2, float rh );

}