// GdiPlusInitializer.h: interface for the CGdiPlusInitializer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GDIPLUSINITIALIZER_H__3A329D0A_3A38_4391_96C2_1D74849C6401__INCLUDED_)
#define AFX_GDIPLUSINITIALIZER_H__3A329D0A_3A38_4391_96C2_1D74849C6401__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CGdiPlusInitializer  
{
private:
	DWORD_PTR	m_dwGdiplusToken;
	bool	m_bIsInit;
public:
	CGdiPlusInitializer();
	~CGdiPlusInitializer();

	bool IsInitialize();

};

extern CGdiPlusInitializer	_GdiPlusInitializer;


#endif // !defined(AFX_GDIPLUSINITIALIZER_H__3A329D0A_3A38_4391_96C2_1D74849C6401__INCLUDED_)
