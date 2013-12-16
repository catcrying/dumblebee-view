// ExcelExporter.h: interface for the CxExcelExporter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXCELEXPORTER_H__CB733E62_4D78_42C5_B317_DEC72321AFEE__INCLUDED_)
#define AFX_EXCELEXPORTER_H__CB733E62_4D78_42C5_B317_DEC72321AFEE__INCLUDED_

#include <DB/xADODB.h>
#include <String/xString.h>

#pragma comment(lib, "ODBCCP32.LIB")

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxExcelExporter  
{
protected:
	CxString GetExcelDriver();

	CxADOConnection m_ADOExcel;	
public:
	CxExcelExporter();
	virtual ~CxExcelExporter();

	BOOL Open( LPCTSTR lpszFileName );
	BOOL ExecuteSQL( LPCTSTR lpszSQL );
#ifdef _UNICODE
	BOOL Open( LPCSTR lpszFileName );
	BOOL ExecuteSQL( LPCSTR lpszSQL );
#else
	BOOL Open( LPCWSTR lpszFileName );
	BOOL ExecuteSQL( LPCWSTR lpszSQL );
#endif
	BOOL Close();
};

#endif // !defined(AFX_EXCELEXPORTER_H__CB733E62_4D78_42C5_B317_DEC72321AFEE__INCLUDED_)
