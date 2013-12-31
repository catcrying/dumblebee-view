/****************************************************************************
*																			*		 
*								GuiToolKit  								*	
*							 (MFC extension)								*			 
* Created by Francisco Campos G. www.beyondata.com fcampos@beyondata.com	*
*--------------------------------------------------------------------------*		   
*																			*
* This program is free software;so you are free to use it any of your		*
* applications (Freeware, Shareware, Commercial),but leave this header		*
* intact.																	*
*																			*
* These files are provided "as is" without warranty of any kind.			*
*																			*
*			       GuiToolKit is forever FREE CODE !!!!!					*
*																			*
*--------------------------------------------------------------------------*
* Created by: Francisco Campos G.											*
* Bug Fixes and improvements : (Add your name)								*
* -Francisco Campos														*				
* Chenlu Li (wikilee@263.net)																			*	
****************************************************************************/



#include "stdafx.h"
#define _WIN32_DCOM
#include <objbase.h>
#include "xADODB.h"

#include <XUtil/xTime.h>

//---------------------------------------------------------------------------------------

CxString GetError(_com_error &e)
{	
	CxString MsgBug;
	_bstr_t Source(e.Source());
	_bstr_t Description(e.Description());
	MsgBug.Format( _T("ADODB Error Source = %s\nDescription= %s\nErrorCode=0x%X"),(LPCTSTR)Source, (LPCTSTR)Description, e.Error() );
#ifdef _DEBUG
	XTRACE( MsgBug );
#endif	
	return MsgBug;
}


CxString GetTypeVar(_variant_t vt)
{
	switch (vt.vt)
	{
	case VT_EMPTY :
		return _T("VT_EMPTY");
		break;
	case   VT_NULL :
		return _T("VT_NULL");
		break;
	case VT_I2:
		return _T("int");
		break;
	case VT_I4 :
		return _T("long");
		break;
	case  VT_R4 :
		return _T("float");
		break;
	case VT_R8 :
		return _T("double");
		break;
	case VT_CY :
		return _T("currency");
		break;
	case VT_DATE:
		return _T("date");
		break;
	case  VT_BSTR :
		return _T("string");
		break;
	case VT_DISPATCH :
		return _T("dispatch");
		break;
	case  VT_ERROR :
		return _T("error");
		break;
	case VT_BOOL :
		return _T("bool");
		break;
	case VT_VARIANT :
		return _T("variant");
		break;
	case VT_UNKNOWN :
		return _T("unknown");
		break;
	case   VT_DECIMAL :
		return _T("decimal");
		break;
	default:
		return _T("");
	}
}

CxADOMDBCompressor::CxADOMDBCompressor()
{
	HRESULT hResult = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED );

}

CxADOMDBCompressor::~CxADOMDBCompressor()
{
}

BOOL CxADOMDBCompressor::Compress( LPCTSTR lpszMDBPathName )
{
	IJetEnginePtr jet(__uuidof(JetEngine));
	CxString strTempFile;
	CxString strMDBFile;
	strMDBFile = lpszMDBPathName;
	strTempFile = lpszMDBPathName;
	strTempFile += _T("__");

	::DeleteFile( strTempFile );

	CxString strSrc, strDest;
	strSrc.Format( _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s;"), strMDBFile );
	strDest.Format( _T("Provider=Microsoft.Jet.OLEDB.4.0;Data Source=%s; Jet OLEDB:Engine Type=5;"), strTempFile );
	
	BSTR bstrSrc	= strSrc.AllocSysString();
	BSTR bstrDest	= strDest.AllocSysString();
	try
	{
		jet->CompactDatabase( bstrSrc, bstrDest );
	} 
	catch( _com_error &e )
	{
		::SysFreeString( bstrSrc );
		::SysFreeString( bstrDest );
		XTRACE( _T("%s"), GetError(e) );
		return FALSE;
	}

	::SysFreeString( bstrSrc );
	::SysFreeString( bstrDest );

	::DeleteFile( strMDBFile );
	::MoveFile( strTempFile, strMDBFile );

	return TRUE;
}

CxADOConnection::CxADOConnection(void)
{
	HRESULT hResult = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED );
	m_pConn.CreateInstance(__uuidof(Connection));
	
}
CxADOConnection::~CxADOConnection(void)
{
	Close();
	m_pConn.Release();
	m_pConn = NULL;
	
}

BOOL CxADOConnection::Open(LPCTSTR ConnectionString,LPCTSTR UID,LPCTSTR PWD,long Options )
{
	HRESULT re;
	try
	{	
		re = m_pConn->Open(_bstr_t(ConnectionString), _bstr_t(UID), _bstr_t(PWD), Options);
		return re == S_OK;
	}
	catch(_com_error &e)
	{
		m_strLastError = GetError(e);
		m_hrLastResult = e.Error();
		return FALSE;
	}
	
}

BOOL CxADOConnection::IsConnect()
{
	return (BOOL) m_pConn->GetState() != adStateClosed;
}



CxADORecordSet* CxADOConnection::Execute(LPCTSTR CommandText,long Options)
{
	CxADORecordSet* m_rs= new CxADORecordSet();
	try
	{
		m_rs->Open(m_pConn,CommandText,adOpenUnspecified,adLockUnspecified, Options);
	}
	catch(_com_error &e)
	{
		GetError(e);
		return NULL;
	}
	return m_rs;
}

long CxADOConnection::BeginTrans()
{
	return m_pConn->BeginTrans();
}

void CxADOConnection::RollbackTrans()
{
	m_pConn->RollbackTrans();
}

void CxADOConnection::CommitTrans()
{
	m_pConn->CommitTrans();
}

void CxADOConnection::Cancel()
{
	m_pConn->Cancel();
}

void CxADOConnection::Close()
{
	if (IsConnect()) m_pConn->Close();
}

void CxADOConnection::SetConnectionTimeout(long ConnectionTimeout)
{
	m_pConn->ConnectionTimeout=ConnectionTimeout;
}

void CxADOConnection::SetConectionString( LPCTSTR ConnectionString)
{
	m_pConn->ConnectionString=ConnectionString;
}

long CxADOConnection::GetConnectionTimeout()
{
	return m_pConn->ConnectionTimeout;
}

LPCTSTR CxADOConnection::GetConectionString()
{
	return m_pConn->ConnectionString;
}


//***********************************************************************************************
CxADORecordSet::CxADORecordSet(void)
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	m_rs.CreateInstance(__uuidof(Recordset));	
	m_Criteria=_T("");
}
CxADORecordSet::~CxADORecordSet(void)
{
	Close();
	if (m_rs) m_rs.Release();
	m_rs=NULL;
}

void CxADORecordSet::Attach(_RecordsetPtr m_prs )
{
	m_rs=m_prs;
}

BOOL CxADORecordSet::Open(_ConnectionPtr ActiveConnection,LPCTSTR Source, CursorTypeEnum CursorType,LockTypeEnum LockType, long Options)
{
	try
	{
		m_rs->Open(Source, _variant_t((IDispatch*)ActiveConnection, TRUE), 
			adOpenStatic, LockType, Options);
	}
	catch(_com_error &e)
	{
		m_hrLastResult = e.Error();
		m_strLastError = GetError(e);
		return FALSE;
	}
	return TRUE;
}



BOOL CxADORecordSet::FindFirst(LPCTSTR Criteria)
{
	MoveFirst();
   	return Find(Criteria,0,adSearchForward,"");
}

void CxADORecordSet::Move(long NumRecords , _variant_t Start)
{
	m_rs->Move(NumRecords,vtMissing);
}

BOOL CxADORecordSet::FindNext()
{
	return Find(m_Criteria,1,adSearchForward,vtPointer);
}


BOOL CxADORecordSet::Find(LPCTSTR Criteria , long SkipRecords , 
					   SearchDirectionEnum SearchDirection,_variant_t Start)
{
	CxString szCri=Criteria;
	
	if (!szCri.IsEmpty())
		m_Criteria=Criteria;
	else
		return FALSE;
	try
	{
		m_rs->Find(_bstr_t(Criteria),SkipRecords,SearchDirection,Start);
		if (SearchDirection ==adSearchForward)
		{
			if (!IsEof())
			{
				vtPointer= m_rs->Bookmark;
				return TRUE;
			}
		}
		else if (SearchDirection ==adSearchBackward)
		{
			if (!IsBof())
			{
				vtPointer= m_rs->Bookmark;
				return TRUE;
			}
		}
		else return FALSE;
		
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	return FALSE;
}



BOOL CxADORecordSet::SetValue(LPCTSTR lpName,CxString szCad)
{
	_variant_t vt;
	if (!szCad.IsEmpty())
	{
		vt.vt = VT_BSTR;
		vt.bstrVal = szCad.AllocSysString();
	}
	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,long lVal)
{
	_variant_t vt;
	vt.lVal=lVal;
	vt.vt=VT_I4;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,unsigned char usVal)
{
	_variant_t vt;
	vt.bVal=usVal;
	vt.vt=VT_UI1;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,short shVal)
{
	_variant_t vt;
	vt.iVal=shVal;
	vt.vt=VT_I2;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,float flVal)
{
	_variant_t vt;
	vt.fltVal=flVal;
	vt.vt=VT_R4;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,double dblVal)
{
	_variant_t vt;
	vt.dblVal=dblVal;
	vt.vt=VT_R8;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,BOOL blVal)
{
	_variant_t vt;
	vt.boolVal=blVal;
	vt.vt=VT_BOOL;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,CxOleDateTime dtVal)
{
	_variant_t vt;
	vt.date=dtVal;
	vt.vt=VT_DATE;	
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName,unsigned long ulVal)
{
	_variant_t vt;
	vt.vt = VT_UI4;
	vt.ulVal = ulVal;
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetValue(LPCTSTR lpName, CxOleCurrency cuVal)
{
	_variant_t vt;
	vt.vt = VT_CY;
	vt.cyVal = cuVal.m_cur;
	if (cuVal.m_status == CxOleCurrency::invalid)
		return FALSE;
	return SetFieldValue(lpName, vt);
}

BOOL CxADORecordSet::SetFieldValue(LPCTSTR lpName, _variant_t vtField)
{
	try
	{
		m_rs->Fields->GetItem(lpName)->Value = vtField; 
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;	
	}
}


CxString CxADORecordSet::GetCollect(LPCTSTR lpField)
{
	try
	{
		_variant_t vt = m_rs->Fields->GetItem(lpField)->Value;
		return (CxString) vt.bstrVal;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return _T("");
	}
	
}

BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,int& nValue)
{
	_variant_t vt;
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_I2)
		{
			nValue=vt.intVal;
			return TRUE;
		}
		else if (vt.vt==VT_BOOL)
		{
			nValue=vt.boolVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}
BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,long& lVal)
{
	_variant_t vt;
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_I4)
		{
			lVal=vt.lVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}
BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,double& dbVal)
{
	_variant_t vt;
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_R8)
		{
			dbVal=vt.dblVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}
BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,CxString& strValue)
{
	_variant_t vt;
	try
	{	
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_BSTR)
		{
			strValue=vt.bstrVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,CxOleCurrency& cyVal)
{
	_variant_t vt;
	vt = m_rs->Fields->GetItem(lpField)->Value;
	
	try
	{
		if (vt.vt==VT_CY)
		{
			cyVal.m_cur=vt.cyVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,CxOleDateTime& dtVal)
{
	_variant_t vt;
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_DATE)
		{
			dtVal=vt.date;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}
BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,float& flVal)
{
	_variant_t vt;
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		if (vt.vt==VT_R4)
		{
			flVal=vt.fltVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADORecordSet::GetCollect(LPCTSTR lpField,_variant_t& vt)
{
	try
	{
		vt = m_rs->Fields->GetItem(lpField)->Value;
		return TRUE;
	}
	catch(_com_error& e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADORecordSet::GetCollect(int nIndex,int& nValue)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_I2)
		{
			nValue=vt.intVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADORecordSet::GetCollect(int nIndex,long& lVal)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_I4)
		{
			lVal=vt.lVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADORecordSet::GetCollect(int nIndex,double& dbVal)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_R8)
		{
			dbVal=vt.dblVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADORecordSet::GetCollect(int nIndex,CxString& strValue)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{	
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_BSTR)
		{
			strValue=vt.bstrVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADORecordSet::GetCollect(int nIndex,CxOleCurrency& cyVal)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_CY;
	try
	{
		vtn.iVal =nIndex;
		vt	= m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_CY)
		{
			cyVal.m_cur=vt.cyVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADORecordSet::GetCollect(int nIndex,CxOleDateTime& dtVal)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_DATE)
		{
			dtVal=vt.date;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADORecordSet::GetFormatDate(LPCTSTR lpField,CxString& m_szDate, CxString Format)
{
	CxOleDateTime time;
	if (!GetCollect(lpField,time)) return FALSE;
	CxTime ct(time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond()); 
	m_szDate =ct.Format(Format);
	return TRUE;
}

BOOL CxADORecordSet::GetFormatDate(int nIndex,CxString& m_szDate, CxString Format)
{
	CxOleDateTime time;
	if (!GetCollect(nIndex,time)) return FALSE;
	CxTime ct(time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond()); 
	m_szDate =ct.Format(Format);
	return TRUE;
}

BOOL CxADORecordSet::GetCollect(int nIndex,float& flVal)
{
	_variant_t vt;
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		if (vt.vt==VT_R4)
		{
			flVal=vt.fltVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}



BOOL CxADORecordSet::GetCollect(int nIndex,_variant_t& vt)
{
	
	_variant_t vtn;
	vtn.vt = VT_I2;
	try
	{
		vtn.iVal = nIndex;
		vt = m_rs->Fields->GetItem(vtn)->Value;
		return TRUE;
	}
	catch(_com_error& e)
	{
		GetError(e);
		return FALSE;
	}
}




BOOL CxADORecordSet::SetFilter(LPCTSTR lpFilter)
{
	if (!IsOpen()) return FALSE;
	try
	{
		m_rs->PutFilter(lpFilter);
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADORecordSet::SetSort(LPCTSTR lpSort)
{
	if (!IsOpen()) return FALSE;
	try
	{
		m_rs->PutSort(lpSort);
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADORecordSet::IsOpen()
{
	if (m_rs)
		return m_rs-> GetState() != adStateClosed;
	return FALSE;
}

void CxADORecordSet::Close()
{
	if (IsOpen())
		m_rs->Close();	
	
}


void CxADORecordSet::MoveFirst()
{
	m_rs->MoveFirst();
}

void CxADORecordSet::MoveLast()
{
	m_rs->MoveLast();
}

void CxADORecordSet::MoveNext()
{
	m_rs->MoveNext();
}

void CxADORecordSet::MovePrevious()
{
	m_rs->MovePrevious();
}

void CxADORecordSet::Cancel()
{
	m_rs->Cancel();	
}

void CxADORecordSet::CancelUpdate()
{
	m_rs->CancelUpdate();	
}

BOOL CxADORecordSet::Delete()
{
	try
	{
		if (m_rs->Delete(adAffectCurrent)== S_OK)
			if (m_rs->Update() ==S_OK)
				return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	return FALSE;
}


BOOL CxADORecordSet::IsEof()
{
	return (BOOL)m_rs->EndOfFile;
}
BOOL CxADORecordSet::IsBof()
{
	return (BOOL)m_rs->BOF;
}

BOOL CxADORecordSet::Supports( CursorOptionEnum CursorOptions ) 
{
	return (BOOL)m_rs->Supports(CursorOptions);
}

void CxADORecordSet::CancelBatch(AffectEnum AffectRecords)
{
	m_rs->CancelBatch(AffectRecords);
}


CxADOField CxADORecordSet::GetField(LPCTSTR lpField)
{
	FieldPtr pField = m_rs->Fields->GetItem(lpField);
	CxADOField Field;
	Field.Attach(pField);
	return Field;
}

CxADOField CxADORecordSet::GetField(int Index)
{
	_variant_t vtIndex;
	vtIndex.vt = VT_I2;
	vtIndex.iVal = Index;
	FieldPtr pField = m_rs->Fields->GetItem(vtIndex);
	CxADOField Field;
	Field.Attach(pField);
	return Field;
}

CxADORecordSet*  CxADORecordSet::Clone(LockTypeEnum LockType)
{
	_RecordsetPtr m_rs1=m_rs->Clone(LockType);
	CxADORecordSet* m_pRs= new CxADORecordSet();
	m_pRs->Attach(m_rs1);
	return m_pRs; 
}

CxADORecordSet* CxADORecordSet::NextRecordset(long RecordsAffected) 
{
	_RecordsetPtr m_rs1=m_rs->NextRecordset((VARIANT*)RecordsAffected);
	CxADORecordSet* m_pRs= new CxADORecordSet();
	m_pRs->Attach(m_rs1);
	return m_pRs; 
}

//***********************************************************************************************************
CxADOField::CxADOField(void)
{
}
CxADOField::~CxADOField(void)
{
}
BOOL CxADOField::SetValue(long lVal)
{
	_variant_t vt;
	vt.lVal=lVal;
	vt.vt=VT_I4;	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}



BOOL CxADOField::SetValue(float flVal)
{
	_variant_t vt;
	vt.fltVal=flVal;
	vt.vt=VT_R4;	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADOField::SetValue(int nVal)
{
	_variant_t vt;
	vt.intVal=nVal;
	vt.vt=VT_I2;	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
	
}

BOOL CxADOField::SetValue(double dbVal)
{
	_variant_t vt;
	vt.dblVal=dbVal;
	vt.vt=VT_R8;	
	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::SetValue(CxString szCad)
{
	_variant_t vt;
	if (!szCad.IsEmpty())
	{
		vt.vt = VT_BSTR;
		vt.bstrVal = szCad.AllocSysString();
	}
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
	
}


BOOL CxADOField::SetValue(bool blVal)
{
	_variant_t vt;
	vt.boolVal=blVal;
	vt.vt=VT_BOOL;	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADOField::SetValue(CxOleDateTime dtVal)
{
	_variant_t vt;
	vt.date=dtVal;
	vt.vt=VT_DATE;	
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::SetValue(_variant_t vt)
{
	try
	{
		field->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::GetValue(int& nVal)
{
	_variant_t vt;
	vt = field->Value;
	try
	{
		if (vt.vt==VT_I2)
		{
			nVal=vt.intVal;
			return TRUE;
		}
		else if (vt.vt==VT_BOOL)
		{
			nVal=vt.boolVal;
			return TRUE;
		}
		else if ( vt.vt==VT_NULL)
		{
			nVal = 0;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::GetValue(CxTime& tmVal )
{
	CxOleDateTime OleDateTime;
	if ( GetValue( OleDateTime ) )
	{
		tmVal = CxTime( 
			OleDateTime.GetYear(), 
			OleDateTime.GetMonth(), 
			OleDateTime.GetDay(),
			OleDateTime.GetHour(),
			OleDateTime.GetMinute(),
			OleDateTime.GetSecond() );

		return TRUE;
	}

	return FALSE;
}

BOOL CxADOField::GetValue(long& lVal)
{
	_variant_t vt;
	vt = field->Value;
	try
	{
		if (vt.vt==VT_I4)
		{
			lVal=vt.lVal;
			return TRUE;
		}
		else if ( vt.vt==VT_NULL)
		{
			lVal = 0;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::GetValue(double& dbVal)
{
	_variant_t vt;
	vt = field->Value;
	try
	{
		if (vt.vt==VT_R8)
		{
			dbVal=vt.dblVal;
			return TRUE;
		}
		else if ( vt.vt==VT_NULL)
		{
			dbVal = 0.;
			return TRUE;
		}		
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADOField::GetValue(CxString& strValue)
{
	_variant_t vt;
	vt =field->Value;
	
	try
	{
		if (vt.vt==VT_BSTR)
		{
			strValue=vt.bstrVal;
			return TRUE;
		}
		else if ( vt.vt==VT_NULL)
		{
			strValue=_T("");
			return TRUE;
		}
		else return FALSE;
	}
	
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADOField::GetValue(CxOleDateTime& dtVal)
{
	_variant_t vt;
	vt = field->Value;
	try
	{
		if (vt.vt==VT_DATE)
		{
			dtVal=vt.date;
			return TRUE;
		}		
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOField::GetValue(float& flVal)
{
	_variant_t vt;
	vt = field->Value;
	try
	{
		if (vt.vt==VT_R4)
		{
			flVal=vt.fltVal;
			return TRUE;
		}
		else if ( vt.vt==VT_NULL)
		{
			flVal = 0.f;
			return TRUE;
		}	
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}



BOOL CxADOField::GetValue(_variant_t& vt)
{
	try
	{
		//_variant_t vt = field->Value;
		vt = field->Value;
		return TRUE;
	}
	catch(_com_error& e)
	{
		GetError(e);
		return FALSE;
	}
}


//***********************************************************************************************************

CxADOParameter::CxADOParameter()
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	pParam.CreateInstance(__uuidof(Parameter));	
}

CxADOParameter::~CxADOParameter()
{
	pParam->Release();
	pParam=NULL;
}


void CxADOParameter::SetAttributes(long Attributes)
{
	pParam->PutAttributes(Attributes);
}

void CxADOParameter::SetDirection(ParameterDirectionEnum Direction)
{
	pParam->PutDirection(Direction);
}

void CxADOParameter::SetName(LPCTSTR szName)
{
	CxString mszname=szName;
	pParam->Name=mszname.AllocSysString();
	
}

void CxADOParameter::SetNumericScale(unsigned char NumericScale)
{
	pParam->PutNumericScale(NumericScale);
}

void CxADOParameter::SetPrecision(unsigned char Precision)
{
	pParam->PutPrecision(Precision);
}

void CxADOParameter::SetSize(long Size)
{
	pParam->PutSize(Size);
}

void CxADOParameter::SetType(DataTypeEnum Type)
{
	pParam->PutType(Type);
}

BOOL CxADOParameter::SetValue(long lVal)
{
	_variant_t vt;
	vt.lVal=lVal;
	vt.vt=VT_I4;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}



BOOL CxADOParameter::SetValue(float flVal)
{
	_variant_t vt;
	vt.fltVal=flVal;
	vt.vt=VT_R4;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADOParameter::SetValue(int nVal)
{
	_variant_t vt;
	vt.intVal=nVal;
	vt.vt=VT_I2;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
	
}

BOOL CxADOParameter::SetValue(double dbVal)
{
	_variant_t vt;
	vt.dblVal=dbVal;
	vt.vt=VT_R8;	
	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::SetValue(CxString szCad)
{
	_variant_t vt;
	if (!szCad.IsEmpty())
	{
		vt.vt = VT_BSTR;
		vt.bstrVal = szCad.AllocSysString();
	}
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
	
}


BOOL CxADOParameter::SetValue(bool blVal)
{
	_variant_t vt;
	vt.boolVal=blVal;
	vt.vt=VT_BOOL;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}


BOOL CxADOParameter::SetValue(CxOleDateTime dtVal)
{
	_variant_t vt;
	vt.date=dtVal;
	vt.vt=VT_DATE;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::SetValue(_variant_t vt)
{
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::GetValue(int& nVal)
{
	_variant_t vt;
	vt = pParam->Value;
	try
	{
		if (vt.vt==VT_I2)
		{
			nVal=vt.intVal;
			return TRUE;
		}
		else if (vt.vt==VT_BOOL)
		{
			nVal=vt.boolVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::GetValue(long& lVal)
{
	_variant_t vt;
	vt = pParam->Value;
	try
	{
		if (vt.vt==VT_I4)
		{
			lVal=vt.lVal;
			return TRUE;
		}
		if (vt.vt==VT_BSTR)
		{
			CxString cad=vt.bstrVal;
			lVal=_tcstol(cad, NULL, 0);
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::SetValue(CxOleCurrency cyVal)
{
	_variant_t vt;
	vt.cyVal=cyVal.m_cur;
	vt.vt=VT_CY;	
	try
	{
		pParam->Value=vt;
		return TRUE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::GetValue(double& dbVal)
{
	_variant_t vt;
	vt = pParam->Value;
	try
	{
		if (vt.vt==VT_R8)
		{
			dbVal=vt.dblVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADOParameter::GetValue(CxString& strValue)
{
	_variant_t vt;
	vt =pParam->Value;
	
	try
	{
		if (vt.vt==VT_BSTR)
		{
			strValue=vt.bstrVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
}

BOOL CxADOParameter::GetValue(CxOleDateTime& dtVal)
{
	_variant_t vt;
	vt = pParam->Value;
	try
	{
		if (vt.vt==VT_DATE)
		{
			dtVal=vt.date;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}

BOOL CxADOParameter::GetValue(CxOleCurrency& cyVal)
{
	_variant_t vt;
	vt = pParam->Value;
	
	try
	{
		if (vt.vt==VT_CY)
		{
			cyVal.m_cur=vt.cyVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}


BOOL CxADOParameter::GetFormatDate(CxString& m_szDate, CxString Format)
{
	CxOleDateTime time;
	if (!GetValue(time)) return FALSE;
	CxTime ct(time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond()); 
	m_szDate =ct.Format(Format);
	return TRUE;
}



BOOL CxADOParameter::GetValue(float& flVal)
{
	_variant_t vt;
	vt = pParam->Value;
	try
	{
		if (vt.vt==VT_R4)
		{
			flVal=vt.fltVal;
			return TRUE;
		}
		else return FALSE;
	}
	catch(_com_error &e)
	{
		GetError(e);
		return FALSE;
	}
	
}



BOOL CxADOParameter::GetValue(_variant_t& vt)
{
	try
	{
		_variant_t vt = pParam->Value;
		return TRUE;
	}
	catch(_com_error& e)
	{
		GetError(e);
		return FALSE;
	}
}


//----------------------------------------------------------------------------------------------

CxADOCommand::CxADOCommand()
{
	::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	pCommand.CreateInstance(__uuidof(Command));	
	
}

CxADOCommand::~CxADOCommand()
{
	Cancel();
	pCommand.Release();
	pCommand=NULL;
}


void CxADOCommand::SetActiveConnection(LPCTSTR szconnec)
{
	m_pCon.Open(szconnec);
	XASSERT( m_pCon.IsConnect());
	SetActiveConnection(&m_pCon);
}

void CxADOCommand::SetActiveConnection(CxADOConnection* pCon)
{
	XASSERT(pCon->IsConnect());
	pCommand->ActiveConnection=pCon->GetConecction();
}

void CxADOCommand::Cancel()
{
	pCommand->Cancel();
} 

void CxADOCommand::SetCommandText(LPCTSTR lpCommand,CommandTypeEnum cmdType)
{
	CxString szCommand=lpCommand;
	pCommand->CommandText=szCommand.AllocSysString();
	pCommand->PutCommandType(cmdType);
}

void CxADOCommand::SetCommandTimeout(long CommandTimeout)
{
	pCommand->PutCommandTimeout(CommandTimeout);
}

void CxADOCommand::SetPrepared(BOOL prepared)
{
	pCommand->PutPrepared((BOOL) prepared);
}

long CxADOCommand::GetState()
{
	return pCommand->GetState();
}

CxADORecordSet* CxADOCommand::Execute(VARIANT* param1,VARIANT* param2,long Options)
{
	try
	{
		_RecordsetPtr m_rs = pCommand->Execute(NULL, NULL, Options);
		CxADORecordSet* m_prs= new CxADORecordSet();
		m_prs->Attach(m_rs);
		return m_prs;
	}
	catch(_com_error &e)
	{
		m_strLastError = GetError(e);
		m_hrLastResult = e.Error();
		return NULL;
	}
	
}

CxADOParameter* CxADOCommand::CreateParameter(CxString Name ,long Size ,
										  DataTypeEnum Type, 
										  ParameterDirectionEnum Direction)
{
	_ParameterPtr param=pCommand->CreateParameter(Name.AllocSysString(), Type,Direction, Size);
	
	CxADOParameter* pParam=new CxADOParameter();
	pParam->Attach(param);
	return pParam;
}

BOOL CxADOCommand::Append(CxADOParameter* pParam)
{
	try
	{
		pCommand->Parameters->Append(pParam->GetParameter());
		return TRUE;
	}
	catch(_com_error& e)
	{
		GetError(e);
		return FALSE;
	}
	
}


