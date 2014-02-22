#include "stdafx.h"
#include <XGraphic/xColorConverter.h>

void xColorConverter::ConvertRGBToHSL( COLORREF rgb, float *H, float *S, float *L )
{
	float delta;
	float r = (float)GetRValue(rgb)/255;
	float g = (float)GetGValue(rgb)/255;
	float b = (float)GetBValue(rgb)/255;   
	float cmax = max(r, max(g, b));
	float cmin = min(r, min(g, b));   
	*L=(cmax+cmin)/2.0f;   

	if(cmax==cmin) 
	{
		*S = 0;      
		*H = 0;
	} 
	else 
	{
		if(*L < 0.5f) 
			*S = (cmax-cmin)/(cmax+cmin);      
		else
			*S = (cmax-cmin)/(2.0f-cmax-cmin);      

		delta = cmax - cmin;
		if(r==cmax) 
			*H = (g-b)/delta;      
		else if(g==cmax)
			*H = 2.0f +(b-r)/delta;
		else          
			*H=4.0f+(r-g)/delta;
		*H /= 6.0f; 
		if(*H < 0.0f)
			*H += 1;  
	}
}

COLORREF xColorConverter::GetRGBFromHLSExtend( float H, float L, float S )
{
	WORD R, G, B; 

	if (S == 0.0)
	{
		R = G = B = unsigned char(L * 255.0);
	}
	else
	{
		float rm1, rm2;

		if (L <= 0.5f)
			rm2 = (float)(L + L * S);
		else
			rm2 = (float)(L + S - L * S);

		rm1 = (float)(2.0f * L - rm2);

		R = xColorConverter::GetRGBFromHue(rm1, rm2, (float)(H + 120.0f));
		G = xColorConverter::GetRGBFromHue(rm1, rm2, (float)(H));
		B = xColorConverter::GetRGBFromHue(rm1, rm2, (float)(H - 120.0f));
	}

	return RGB(R, G, B);
}

COLORREF xColorConverter::GetRGBFromHLS( float H, float L, float S )
{
	float r, g, b;
	float m1, m2;

	if(S==0) 
	{
		r=g=b=L;
	} 
	else 
	{
		if(L <=0.5)
			m2 = L*(1.0f+S);
		else
			m2 = L+S-L*S;
		m1 = 2.0f*L-m2;
		r = xColorConverter::GetRGBFromHue((float)m1, (float)m2, (float)(H+1.0f/3.0f));
		g = xColorConverter::GetRGBFromHue((float)m1, (float)m2, (float)H);
		b = xColorConverter::GetRGBFromHue((float)m1, (float)m2, (float)(H-1.0f/3.0f));
	}
	return RGB((BYTE)(r*255), (BYTE)(g*255), (BYTE)(b*255));
}

BYTE xColorConverter::GetRGBFromHue( float rm1, float rm2, float rh )
{
	if (rh > 360.0f)
		rh -= 360.0f;
	else if (rh < 0.0f)
		rh += 360.0f;

	if (rh <  60.0f)
		rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;   
	else if (rh < 180.0f)
		rm1 = rm2;
	else if (rh < 240.0f)
		rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;      

	return static_cast<BYTE>(rm1 * 255 + .5f);
}