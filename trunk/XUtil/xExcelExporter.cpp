// xExcelExporter.cpp: implementation of the CxExcelExporter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <DB/xExcelExporter.h>

#include <odbcinst.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxExcelExporter::CxExcelExporter()
{

}

CxExcelExporter::~CxExcelExporter()
{
	Close();
}

BOOL CxExcelExporter::Open( LPCTSTR lpszFileName )
{
	Close();

	CxString strExcelDriver = GetExcelDriver();

	if ( strExcelDriver.IsEmpty() ) return FALSE;

	CxString strSQL;
	strSQL.Format( _T("DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s"), strExcelDriver, lpszFileName, lpszFileName );

	return m_ADOExcel.Open( strSQL );
}

BOOL CxExcelExporter::Close()
{
	try
	{
		m_ADOExcel.Close();
	}
	catch ( ... )
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CxExcelExporter::ExecuteSQL( LPCTSTR lpszSQL )
{
	CxADORecordSet* pRS = m_ADOExcel.Execute( lpszSQL );
	if ( pRS )
	{
		delete pRS;
		return TRUE;
	}
	return FALSE;
}

CxString CxExcelExporter::GetExcelDriver()
{
	TCHAR szBuf[2001];
	WORD cbBufMax = 2000;
	WORD cbBufOut;
	TCHAR *pszBuf = szBuf;
	CxString sDriver;	

	if ( !SQLGetInstalledDrivers(szBuf, cbBufMax, &cbBufOut) )
		return CxString("");
	
	do
	{
		if ( _tcsstr( pszBuf, _T("Excel") ) != 0 )
		{
			sDriver = CxString( pszBuf );
			break;
		}

		pszBuf = _tcschr( pszBuf, _T('\0') ) + 1;
	}
	while ( pszBuf[1] != _T('\0') );
	
	return sDriver;
}