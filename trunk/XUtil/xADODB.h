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
 * Modify by HyeongCheol Kim(bluewiz96@gmail.com)                           *	
 ****************************************************************************/

#ifndef __X_ADODB_H__
#define __X_ADODB_H__

#if _MSC_VER > 1000
#pragma once
#endif

#if 1	// for 64bit OS (Win7 64bit...)
	#import "C:\Program Files (x86)\Common Files\System\ado\msado28.tlb" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
	#import "C:\Program Files (x86)\Common Files\System\ado\msjro.dll" no_namespace
#else
	#if 1		// for 32bit OS (Win7 32bit...)
		#import "C:\Program Files\Common Files\System\ado\msado28.tlb" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
		#import "C:\Program Files\Common Files\System\ado\msjro.dll" no_namespace
	#else		// for 32bit OS (XP)
		#pragma warning(disable: 4146)
		#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename_namespace("ADOCG") rename("EOF", "EndOfFile")
		#import "C:\Program Files\Common Files\System\ado\msjro.dll" rename_namespace("ADOX")
		using namespace ADOX;
	#endif
#endif
using namespace ADOCG;

#include "icrsint.h"
#include <XUtil/xOleDateTime.h>
#include <XUtil/xTime.h>

class CxADOMDBCompressor
{
public:
	CxADOMDBCompressor();
	~CxADOMDBCompressor();

	BOOL Compress( LPCTSTR lpszMDBPathName );
};

class CxADOField
{
protected:
	FieldPtr field;
public:
	CxADOField(void);
	~CxADOField(void);
public:

	FieldPtr GetFieldPtr() { return field; }
	void Attach(FieldPtr mField) { field=mField; }
	CxString GetName() { CxString Name=(LPCTSTR)field->GetName(); return Name; }
	short   GetType() { return field->GetType(); }
	long    GetActualSize() { return field->GetActualSize(); }
	long    GetDefinedSize() { return field->GetDefinedSize(); }
	long    GetAttributes() { return field->GetAttributes(); }
	CxString GetOriginalVal() { _variant_t vt=field->GetOriginalValue(); return (CxString) vt.bstrVal; }
	BOOL	SetValue(long lVal);
	BOOL	SetValue(float flVal);
	BOOL	SetValue(int nVal);
	BOOL	SetValue(double dbVal);
	BOOL	SetValue(CxString szValue);
	BOOL	SetValue(bool blVal);
	BOOL	SetValue(CxOleDateTime dtVal);
	BOOL	SetValue(_variant_t vt);
	BOOL	GetValue(int& nValue);
	BOOL	GetValue(long& lVal);
	BOOL	GetValue(double& dbVal);
	BOOL	GetValue(CxString& strValue);
	BOOL	GetValue(CxOleDateTime& dtVal);
	BOOL	GetValue(float& flVal);
	BOOL	GetValue(_variant_t& vtVal);
	BOOL	GetValue(CxTime& tmVal );
	CxString GetUnderlyingValue() { _variant_t vt=field->UnderlyingValue; return (CxString)vt.bstrVal; }
};

class CxADOCommand;
class CxADOConnection;
class CxADORecordSet
{
public:
	CxADORecordSet(void);
	~CxADORecordSet(void);

protected:
	_RecordsetPtr m_rs;
	CxADOConnection* m_pCon;
	CxString	m_strLastError;
	HRESULT		m_hrLastResult;
public:
	BOOL Open(_ConnectionPtr ActiveConnection,LPCTSTR Source= _T(""), CursorTypeEnum CursorType = adOpenUnspecified,LockTypeEnum LockType= adLockUnspecified, long Options = -1);
	void MoveFirst();
	void MoveLast();
	void MoveNext();
	void MovePrevious();
	void Cancel();
	void CancelUpdate(); 
	void Close();
	BOOL IsEof();
	BOOL IsBof();
	long GetRecordCount() { return m_rs->RecordCount; }
	long GetAbsolutePage() { return m_rs->GetAbsolutePage(); }
	void SetAbsolutePage(int nPage)	{ m_rs->PutAbsolutePage((enum PositionEnum)nPage); }
	long GetPageCount()	{ return m_rs->GetPageCount(); }
	long GetPageSize()	{ return m_rs->GetPageSize(); }
	void SetPageSize(int nSize)	{ m_rs->PutPageSize(nSize); }
	long GetAbsolutePosition()	{ return m_rs->GetAbsolutePosition(); }
	void SetAbsolutePosition(int nPosition)	{ m_rs->PutAbsolutePosition((enum PositionEnum)nPosition); }
	BOOL Find(LPCTSTR Criteria, long SkipRecords=0, SearchDirectionEnum SearchDirection= adSearchForward, _variant_t Start=vtMissing);
	long GetNumFields()	{ return m_rs->Fields->GetCount(); }
	_RecordsetPtr GetAdoRecordSet() { return m_rs; }
	//void SetBookmark(){m_rs->Bookmark = m_rs->GetBookmark();};

	CxString GetCollect(LPCTSTR lpField);
	
	BOOL GetCollect(LPCTSTR lpField,int& nValue);
	BOOL GetCollect(LPCTSTR lpField,long& lVal);
	BOOL GetCollect(LPCTSTR lpField,double& dbVal);
	BOOL GetCollect(LPCTSTR lpField,CxString& strValue);
	BOOL GetCollect(LPCTSTR lpField,CxOleDateTime& dtVal);
	BOOL GetCollect(LPCTSTR lpField,float& flVal);
	BOOL GetCollect(LPCTSTR lpField,bool& blVal);
	BOOL GetCollect(LPCTSTR lpField,_variant_t& vt);
	BOOL GetCollect(LPCTSTR lpField,CxOleCurrency& cyVal);
	
	BOOL GetCollect(int nIndex,int& nValue);
	BOOL GetCollect(int nIndex,long& lVal);
	BOOL GetCollect(int nIndex,double& dbVal);
	BOOL GetCollect(int nIndex,CxString& strValue);
	BOOL GetCollect(int nIndex,CxOleDateTime& dtVal);
	BOOL GetCollect(int nIndex,float& flVal);
	BOOL GetCollect(int nIndex,bool& blVal);
	BOOL GetCollect(int nIndex,_variant_t& vt);
	BOOL GetCollect(int nIndex,CxOleCurrency& cyVal);

	BOOL GetFormatDate(int nIndex,CxString& m_szDate, CxString Format=_T("%d/%m/%Y")); 
	BOOL GetFormatDate(LPCTSTR lpField,CxString& m_szDate, CxString Format=_T("%d/%m/%Y"));
	CxADOField GetField(LPCTSTR lpField);
	CxADOField GetField(int Index);
	BOOL SetValue(LPCTSTR lpName,CxString szCad);
	BOOL SetValue(LPCTSTR lpName,long lVal);
	BOOL SetValue(LPCTSTR lpName,unsigned char usVal);
	BOOL SetValue(LPCTSTR lpName,short shVal);
	BOOL SetValue(LPCTSTR lpName,float flVal);
	BOOL SetValue(LPCTSTR lpName,double dblVal);
	BOOL SetValue(LPCTSTR lpName,BOOL blVal);
	BOOL SetValue(LPCTSTR lpName,CxOleDateTime dtVal);
	BOOL SetValue(LPCTSTR lpName,unsigned long ulVal);
	BOOL SetValue(LPCTSTR lpName, CxOleCurrency cuVal);
	
    BOOL SetFieldValue(LPCTSTR lpName, _variant_t vt);
	BOOL Supports( CursorOptionEnum CursorOptions ) ;
	BOOL FindFirst(LPCTSTR Criteria);
	BOOL FindNext();
	void CxADORecordSet::Move(long NumRecords , _variant_t Start);
	void Attach(_RecordsetPtr m_prs );
	void CancelBatch(AffectEnum AffectRecords= adAffectAll);
	void SetCacheSize(long lSizeCache) { m_rs->put_CacheSize(lSizeCache); }
	void GetCacheSize() { m_rs->GetCacheSize(); }
	CxADORecordSet*  Clone(LockTypeEnum LockType= adLockUnspecified);
	CxADORecordSet* NextRecordset(long RecordsAffected);
	BOOL AddNew() { return m_rs->AddNew() != S_OK; }
	BOOL Update() { return m_rs->Update() != S_OK; }
	BOOL Delete();
	BOOL IsOpen();
	BOOL SetFilter(LPCTSTR lpFilter);
	BOOL SetSort(LPCTSTR lpSort);
	EditModeEnum GetEditMode() { return m_rs->EditMode; }
	
	CxString& GetLastErrorString() { return m_strLastError; }
	HRESULT GetLastError() { return m_hrLastResult; }
protected:
	_variant_t vtPointer;
	LPCTSTR m_Criteria;
};

class CxADOConnection
{
public:
	CxADOConnection(void);
	~CxADOConnection(void);

protected:
	_ConnectionPtr m_pConn;
	CxString m_strLastError;
	HRESULT		m_hrLastResult;
public:
	BOOL Open(LPCTSTR ConnectionString,LPCTSTR UID=_T(""),LPCTSTR PWD=_T(""),long Options=-1);
	CxADORecordSet* Execute(LPCTSTR CommandText,long Options=-1 );
	long BeginTrans();
	void RollbackTrans();
	void CommitTrans();
	void Cancel();
	void Close();
	BOOL IsConnect();
	void SetConnectionTimeout(long ConnectionTimeout);
	void SetConectionString( LPCTSTR ConnectionString);
	long GetConnectionTimeout();
	_ConnectionPtr GetConecction(){return m_pConn;};
	LPCTSTR GetConectionString();

	CxString& GetLastErrorString() { return m_strLastError; }
	HRESULT GetLastError() { return m_hrLastResult; }

};

class CxADOParameter
{
public:
	CxADOParameter();
	~CxADOParameter();
protected:
	_ParameterPtr pParam;
public:
	void SetAttributes(long Attributes);
	void SetDirection(ParameterDirectionEnum Direction=adParamUnknown);
	void SetName(LPCTSTR szName);
	void SetNumericScale(unsigned char NumericScale);
	void SetPrecision(unsigned char Precision);
	void SetSize(long Size);
	void SetType(DataTypeEnum Type);
	BOOL SetValue(long lVal);
	BOOL SetValue(float flVal);
	BOOL SetValue(int nVal);
	BOOL SetValue(double dbVal);
	BOOL SetValue(CxString szValue);
	BOOL SetValue(bool blVal);
	BOOL SetValue(CxOleDateTime dtVal);
	BOOL SetValue(CxOleCurrency cyVal);
	BOOL SetValue(_variant_t vt);
	BOOL GetValue(int& nValue);
	BOOL GetValue(long& lVal);
	BOOL GetValue(double& dbVal);
	BOOL GetValue(CxString& strValue);
	BOOL GetValue(CxOleDateTime& dtVal);
	BOOL GetValue(float& flVal);
	BOOL GetValue(_variant_t& vtVal);
	BOOL GetValue(CxOleCurrency& cyVal);
	BOOL GetFormatDate(CxString& m_szDate, CxString Format=_T("%d/%m/%Y"));
	_ParameterPtr GetParameter(){return pParam;};
	void Attach(_ParameterPtr param) {pParam=param;};

	void AppendChunk(_variant_t& vtVal) { pParam->AppendChunk(vtVal); }

};

class CxADOCommand
{
public:
	CxADOCommand();
	~CxADOCommand();
protected:
	_CommandPtr	pCommand;
	CxADOConnection m_pCon;
	CxString m_strLastError;
	HRESULT		m_hrLastResult;
public:
	void SetActiveConnection(CxADOConnection* pCon);
	void SetActiveConnection(LPCTSTR szconnec);
	void Cancel();
	void SetCommandText(LPCTSTR lpCommand,CommandTypeEnum cmdType=adCmdText);
	void SetCommandTimeout(long CommandTimeout);
	void SetPrepared(BOOL prepared);
	long GetState();
	
	CxADORecordSet* Execute(VARIANT* param1=NULL,VARIANT* param2=NULL,long Options=-1);
	CxADOParameter* CreateParameter(CxString Name ,long Size ,DataTypeEnum Type= adEmpty, 
					ParameterDirectionEnum Direction= adParamInput) ;
	BOOL Append(CxADOParameter* pParam);
	_CommandPtr GetCommand(){return pCommand;};
	void Attach(_CommandPtr	Command) {pCommand=Command;};

	CxString& GetLastErrorString() { return m_strLastError; }
	HRESULT GetLastError() { return m_hrLastResult; }
	
};

#endif // __X_ADODB_H__