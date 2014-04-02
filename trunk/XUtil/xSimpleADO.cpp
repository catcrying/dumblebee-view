#include "stdafx.h"
#include <XUtil/DB/xSimpleADO.h>
#include "xADODB.h"
//#include <Strsafe.h>
#include <vector>

class CxADOParameterSet
{
public:
	struct ParameterSet
	{
		CxADOParameter* pParameter;
		CxString		strName;
		variant_t		var;
		int				size;
	};
protected:
	std::vector<ParameterSet*>	m_Parameters;
public:
	void AddParameter(ParameterSet* pParameter)
	{
		m_Parameters.push_back(pParameter);
	}
	void Clear()
	{
		for (std::vector<ParameterSet*>::iterator iter = m_Parameters.begin() ; iter != m_Parameters.end() ; ++iter)
		{
			if ((*iter)->pParameter)
				delete (*iter)->pParameter;
			delete *iter;
		}
		m_Parameters.clear();
	}
	int GetCount()
	{
		return (int)m_Parameters.size();
	}
	ParameterSet* GetAt(int index)
	{
		if (index < 0 || index >= (int)m_Parameters.size())
			return NULL;
		return m_Parameters.at(index);
	}
	CxADOParameterSet()
	{
	}
	~CxADOParameterSet()
	{
		Clear();
	}
};

CxSimpleADO::IFieldPtr::IFieldPtr()
{
	m_pnRef = NULL;
	m_pField = NULL;
}

CxSimpleADO::IFieldPtr::IFieldPtr(CxADOField* pPtr)
{
	m_pnRef = new int(1);
	m_pField = pPtr;
	XTRACE( _T("Create Field: %p\n"), m_pField );
}

CxSimpleADO::IFieldPtr::IFieldPtr(const IFieldPtr& cother)
{
	IFieldPtr& other = const_cast<IFieldPtr&> (cother);
	m_pnRef = other.m_pnRef;
	m_pField = other.m_pField;
	if (m_pField)
		++(*m_pnRef);
}

const CxSimpleADO::IFieldPtr& CxSimpleADO::IFieldPtr::operator = ( const IFieldPtr& cother )
{
	IFieldPtr& other = const_cast<IFieldPtr&> (cother);
	m_pnRef = other.m_pnRef;
	m_pField = other.m_pField;
	if (m_pField)
		++(*m_pnRef);
	return *this;
}

CxSimpleADO::IFieldPtr::~IFieldPtr()
{
	if (m_pField)
		--(*m_pnRef);
	if (*m_pnRef == 0)
	{
		XTRACE( _T("Destroy Field: %p\n"), m_pField );
		if (m_pField)
			delete m_pField;
		m_pField = NULL;
		delete m_pnRef;
		m_pnRef = NULL;
	}
}

BOOL CxSimpleADO::IFieldPtr::IsValid()
{
	return m_pField ? TRUE : FALSE;
}

BOOL CxSimpleADO::IFieldPtr::GetValue(int& nValue)
{
	return m_pField->GetValue(nValue);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(long& lVal)
{
	return m_pField->GetValue(lVal);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(double& dbVal)
{
	return m_pField->GetValue(dbVal);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(CxString& strValue)
{
	return m_pField->GetValue(strValue);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(CxOleDateTime& dtVal)
{
	return m_pField->GetValue(dtVal);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(float& flVal)
{
	return m_pField->GetValue(flVal);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(_variant_t& vtVal)
{
	return m_pField->GetValue(vtVal);
}

BOOL CxSimpleADO::IFieldPtr::GetValue(CxTime& tmVal )
{
	return m_pField->GetValue(tmVal);
}

LPWSTR CxSimpleADO::IFieldPtr::GetName()
{
	static WCHAR wszName[256];
	wszName[0] = L'\0';
	CxString strName = m_pField->GetName();
	USES_CONVERSION;
	WCHAR* pName = T2W((LPTSTR)(LPCTSTR)strName);
	wcscpy_s(wszName, pName);
	return wszName;
}

short CxSimpleADO::IFieldPtr::GetType()
{
	return m_pField->GetType();
}

long CxSimpleADO::IFieldPtr::GetActualSize()
{
	return m_pField->GetActualSize();
}

long CxSimpleADO::IFieldPtr::GetDefinedSize()
{
	return m_pField->GetDefinedSize();
}

long CxSimpleADO::IFieldPtr::GetAttributes()
{
	return m_pField->GetAttributes();
}

CxSimpleADO::IRecordSetPtr::IRecordSetPtr(CxADOCommand*& pCmd) : m_pCommand(pCmd)
{
	m_pnRef = NULL;
	m_pRecordSet = NULL;
}

CxSimpleADO::IRecordSetPtr::IRecordSetPtr(CxADOCommand*& pCmd, CxADORecordSet* pPtr) : m_pCommand(pCmd)
{
	m_pnRef = new int(1);
	m_pRecordSet = pPtr;
	XTRACE( _T("Create RecordSet: %p\n"), m_pRecordSet );
}

CxSimpleADO::IRecordSetPtr::IRecordSetPtr(const IRecordSetPtr& cother) : m_pCommand(cother.m_pCommand)
{
	IRecordSetPtr& other = const_cast<IRecordSetPtr&> (cother);
	m_pnRef = other.m_pnRef;
	m_pRecordSet = other.m_pRecordSet;
	if (m_pRecordSet)
		++(*m_pnRef);
}

const CxSimpleADO::IRecordSetPtr& CxSimpleADO::IRecordSetPtr::operator = ( const IRecordSetPtr& cother )
{
	IRecordSetPtr& other = const_cast<IRecordSetPtr&> (cother);
	m_pnRef = other.m_pnRef;
	m_pRecordSet = other.m_pRecordSet;
	if (m_pRecordSet)
		++(*m_pnRef);
	return *this;
}

CxSimpleADO::IRecordSetPtr::~IRecordSetPtr()
{
	if (m_pRecordSet)
		--(*m_pnRef);
	if (*m_pnRef == 0)
	{
		XTRACE( _T("Destroy RecordSet: %p\n"), m_pRecordSet );
		if (m_pRecordSet)
			delete m_pRecordSet;
		m_pRecordSet = NULL;
		delete m_pnRef;
		m_pnRef = NULL;
	}
	XTRACE( _T("Reset Command\n") );
	if (m_pCommand)
	{
		delete m_pCommand;
		m_pCommand = new CxADOCommand();
	}
}

BOOL CxSimpleADO::IRecordSetPtr::IsValid()
{
	return m_pRecordSet ? TRUE : FALSE;
}

void CxSimpleADO::IRecordSetPtr::MoveFirst()
{
	m_pRecordSet->MoveFirst();
}

void CxSimpleADO::IRecordSetPtr::MoveLast()
{
	m_pRecordSet->MoveLast();
}

void CxSimpleADO::IRecordSetPtr::MoveNext()
{
	m_pRecordSet->MoveNext();
}

void CxSimpleADO::IRecordSetPtr::MovePrevious()
{
	m_pRecordSet->MovePrevious();
}

BOOL CxSimpleADO::IRecordSetPtr::IsEof()
{
	return m_pRecordSet->IsEof();
}

BOOL CxSimpleADO::IRecordSetPtr::IsBof()
{
	return m_pRecordSet->IsBof();
}

long CxSimpleADO::IRecordSetPtr::GetRecordCount()
{
	return m_pRecordSet->GetRecordCount();
}

long CxSimpleADO::IRecordSetPtr::GetNumFields()
{
	return m_pRecordSet->GetNumFields();
}

CxSimpleADO::IFieldPtr CxSimpleADO::IRecordSetPtr::GetField(LPCWSTR lpField)
{
	USES_CONVERSION;
	CxADOField* pField = NULL;
	try
	{
		CxADOField field = m_pRecordSet->GetField(W2T((LPWSTR)lpField));
		pField = new CxADOField(field);
	}
	catch (_com_error& e)
	{
		CxString strError;
		strError.Format( _T("DB FIELD Error! Code: %08lx, Description: %s\n"), e.Error(), (LPCTSTR)(e.Description()) );
		XTRACE( strError );
	}

	return IFieldPtr(pField);
}

CxSimpleADO::IFieldPtr CxSimpleADO::IRecordSetPtr::GetField(int index)
{
	CxADOField* pField = NULL;
	try
	{
		CxADOField field = m_pRecordSet->GetField(index);
		pField = new CxADOField(field);
	}
	catch (_com_error& e)
	{
		CxString strError;
		strError.Format( _T("DB FIELD Error! Code: %08lx, Description: %s\n"), e.Error(), (LPCTSTR)(e.Description()) );
		XTRACE( strError );
	}
	return IFieldPtr(pField);
}

CxSimpleADO::CxSimpleADO(void)
{
	m_pParameterSet = new CxADOParameterSet();
	m_pCommand = new CxADOCommand();
	m_pConnection = new CxADOConnection();
}


CxSimpleADO::~CxSimpleADO(void)
{
	if (m_pParameterSet)
	{
		delete m_pParameterSet;
		m_pParameterSet = NULL;
	}
	if (m_pConnection)
	{
		delete m_pConnection;
		m_pConnection = NULL;
	}
	if (m_pCommand)
	{
		delete m_pCommand;
		m_pCommand = NULL;
	}
}

BOOL CxSimpleADO::Connect(LPCWSTR lpszConnectionString)
{
	USES_CONVERSION;
	return m_pConnection->Open( W2T((LPWSTR)lpszConnectionString) );
}

void CxSimpleADO::Disconnect()
{
	m_pConnection->Close();
}

BOOL CxSimpleADO::IsConnect()
{
	return m_pConnection->IsConnect();
}

void CxSimpleADO::SetConnectionTimeout(long ConnectionTimeout)
{
	m_pConnection->SetConnectionTimeout(ConnectionTimeout);
}

long CxSimpleADO::GetConnectionTimeout()
{
	return m_pConnection->GetConnectionTimeout();
}

LPCWSTR CxSimpleADO::GetADOLastErrorString()
{
	static WCHAR wszLastError[512];
	wszLastError[0] = L'\0';
	CxString strError = m_pConnection->GetLastErrorString();
	USES_CONVERSION;
	WCHAR* pError = T2W((LPTSTR)(LPCTSTR)strError);
	wcscpy_s(wszLastError, pError);

	return wszLastError;
}

HRESULT CxSimpleADO::GetLastError()
{
	return m_pConnection->GetLastError();
}

CxSimpleADO::IRecordSetPtr CxSimpleADO::ExecuteDirectSql(LPCWSTR lpszSql)
{
	if (!IsConnect())
		return IRecordSetPtr(m_pCommand, NULL);
	USES_CONVERSION;

	m_pCommand->SetActiveConnection( m_pConnection );
	m_pCommand->SetCommandText( W2T((LPWSTR)lpszSql) );

	CxADORecordSet* pRs = m_pCommand->Execute();
	if (!pRs)
		return IRecordSetPtr(m_pCommand, NULL);

	return IRecordSetPtr(m_pCommand, pRs);
}

void CxSimpleADO::AppendBlobParameter(LPCWSTR lpParamName, _variant_t& vtBlob, int nBlobSize)
{
	USES_CONVERSION;
	CxADOParameterSet::ParameterSet* pParamSet = new CxADOParameterSet::ParameterSet;
	pParamSet->strName = W2T((LPWSTR)lpParamName);
	VariantCopy( &pParamSet->var, &vtBlob );
	pParamSet->size = nBlobSize;
	pParamSet->pParameter = NULL;
	m_pParameterSet->AddParameter(pParamSet);
}

CxSimpleADO::IRecordSetPtr CxSimpleADO::Execute(LPCWSTR lpszSql)
{
	if (!IsConnect())
		return IRecordSetPtr(m_pCommand, NULL);

	USES_CONVERSION;

	m_pCommand->SetActiveConnection( m_pConnection );
	m_pCommand->SetCommandText( W2T((LPWSTR)lpszSql) );

	int nParamCount = m_pParameterSet->GetCount();
	for ( int i=0 ; i<nParamCount ; i++ )
	{
		CxADOParameterSet::ParameterSet* pParamSet = m_pParameterSet->GetAt(i);
		XASSERT( !pParamSet->pParameter );
		pParamSet->pParameter = m_pCommand->CreateParameter( pParamSet->strName, pParamSet->size, adLongVarBinary );
		m_pCommand->Append(pParamSet->pParameter);
		pParamSet->pParameter->AppendChunk(pParamSet->var);
	}

	CxADORecordSet* pRs = m_pCommand->Execute();

	m_pParameterSet->Clear();

	if (!pRs)
		return IRecordSetPtr(m_pCommand, NULL);

	return IRecordSetPtr(m_pCommand, pRs);
}

long CxSimpleADO::BeginTrans()
{
	return m_pConnection->BeginTrans();
}

void CxSimpleADO::RollbackTrans()
{
	m_pConnection->RollbackTrans();
}

void CxSimpleADO::CommitTrans()
{
	m_pConnection->CommitTrans();
}

void CxSimpleADO::Cancel()
{
	m_pConnection->Cancel();
}