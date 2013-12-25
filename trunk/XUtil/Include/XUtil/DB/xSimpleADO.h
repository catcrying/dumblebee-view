#pragma once

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/export.h>

class CxADOConnection;
class CxADOCommand;
class CxADORecordSet;
class CxADOParameterSet;
class CxADOField;
class CxString;
class CxOleDateTime;
class CxTime;
class _variant_t;
class XUTIL_API CxSimpleADO
{
public:
	class IRecordSetPtr;
	class IFieldPtr
	{
		friend class IRecordSetPtr;
	protected:
		CxADOField* m_pField;
		explicit IFieldPtr(CxADOField* pPtr);
	public:
		~IFieldPtr();

		BOOL IsValid();
		BOOL GetValue(int& nValue);
		BOOL GetValue(long& lVal);
		BOOL GetValue(double& dbVal);
		BOOL GetValue(CxString& strValue);
		BOOL GetValue(CxOleDateTime& dtVal);
		BOOL GetValue(float& flVal);
		BOOL GetValue(_variant_t& vtVal);
		BOOL GetValue(CxTime& tmVal );

		LPWSTR GetName();
		short GetType();
		long GetActualSize();
		long GetDefinedSize();
		long GetAttributes();
	};
	class IRecordSetPtr
	{
		friend class CxSimpleADO;
	protected:
		CxADORecordSet* m_pRecordSet;
		explicit IRecordSetPtr(CxADORecordSet* pPtr);
	public:
		~IRecordSetPtr();

		BOOL IsValid();
		void MoveFirst();
		void MoveLast();
		void MoveNext();
		void MovePrevious();
		BOOL IsEof();
		BOOL IsBof();
		long GetRecordCount();
		long GetNumFields();
		IFieldPtr GetField(LPCWSTR lpField);
		IFieldPtr GetField(int index);
	};
protected:
	CxADOConnection*	m_pConnection;
	CxADOCommand*		m_pCommand;
	CxADOParameterSet*	m_pParameterSet;
public:
	CxSimpleADO(void);
	~CxSimpleADO(void);

	BOOL Connect(LPCWSTR lpszConnectionString);

	void Disconnect();
	BOOL IsConnect();
	void SetConnectionTimeout(long ConnectionTimeout);
	long GetConnectionTimeout();

	LPCWSTR GetADOLastErrorString();
	HRESULT GetLastError();

	IRecordSetPtr ExecuteDirectSql(LPCWSTR lpszSql);

	void SetSqlCommand(LPCWSTR lpszSql);
	void AppendBlobParameter(LPCWSTR lpParamName, _variant_t& vtBlob, int nBlobSize);
	IRecordSetPtr Execute();

	long BeginTrans();
	void RollbackTrans();
	void CommitTrans();
	void Cancel();

};

