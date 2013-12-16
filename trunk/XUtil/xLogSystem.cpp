#include "stdafx.h"

#include <XUtil/Log/xLogSystem.h>

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <XUtil/xUtils.h>

//////////////////////////////////////////////////////////////////////
// CxLogSystem
//////////////////////////////////////////////////////////////////////

CxLogSystem::CxLogSystem( LPCTSTR lpszServiceName ) :
	m_nExpirePeriod(-1), m_strServiceName(lpszServiceName), 
	m_bStart(FALSE), 
	m_bMonthlySplitFolder(FALSE), m_bUseTxtFile(TRUE), m_bUseCSVFile(FALSE),
	m_Time(CxTime::GetCurrentTime())
{
}

CxLogSystem::~CxLogSystem()
{
	if (m_bStart)
		Stop();
}

void CxLogSystem::GetLogPathName( CxTime& Time, CxString& strLogPathName )
{
	int nYear = Time.GetYear();
	int nMonth = Time.GetMonth();
	int nDay = Time.GetDay();
	
	strLogPathName.Format( _T("%s\\%s_%04d_%02d_%02d.txt"), m_strLogRootPath, m_strServiceName, nYear, nMonth, nDay );
}

void CxLogSystem::GetCSVPathName( CxTime& Time, CxString& strCSVPathName )
{
	int nYear = Time.GetYear();
	int nMonth = Time.GetMonth();
	int nDay = Time.GetDay();
	
	strCSVPathName.Format( _T("%s\\%s_%04d_%02d_%02d.csv"), m_strLogRootPath, m_strServiceName, nYear, nMonth, nDay );
}

void CxLogSystem::UseCSVFileA( BOOL bUse, LPCSTR lpszCSVHeader )
{
	USES_CONVERSION;
	m_bUseCSVFile = bUse;
	m_strCSVHeader.Format( _T("%s\r\n"), A2T((LPSTR)lpszCSVHeader) );
}

void CxLogSystem::UseCSVFileW( BOOL bUse, LPCWSTR lpszCSVHeader )
{
	USES_CONVERSION;
	m_bUseCSVFile = bUse;
	m_strCSVHeader.Format( _T("%s\r\n"), W2T((LPWSTR)lpszCSVHeader) );
}

void CxLogSystem::UseTextFile( BOOL bUse )
{
	m_bUseTxtFile = bUse;
}

void CxLogSystem::UseMonthlySplitFolder( BOOL bUse )
{
	m_bMonthlySplitFolder = bUse;
}

void CxLogSystem::SetLogDirectoryA( LPCSTR lpszPath )
{
	USES_CONVERSION;
	m_strLogRootPath = A2T((LPSTR)lpszPath);
}

void CxLogSystem::SetLogDirectoryW( LPCWSTR lpszPath )
{
	USES_CONVERSION;
	m_strLogRootPath = W2T((LPWSTR)lpszPath);
}

BOOL CxLogSystem::Start()
{
	if ( m_strLogRootPath.IsEmpty() )
		return FALSE;

	MakeDirectory( m_strLogRootPath );

	DeleteExpiredLogFile();
	DeleteExpiredCSVFile();
	
	m_Time = CxTime::GetCurrentTime();

	CxString strLogPathName, strExcelPathName, strCSVPathName;
	GetLogPathName( m_Time, strLogPathName );
	GetCSVPathName( m_Time, strCSVPathName );
	
	if ( !OpenLogFile(strLogPathName) )
		return FALSE;

	if ( !OpenCSVFile( strCSVPathName ) )
		return FALSE;

	m_bStart = TRUE;
	
	return TRUE;
}

BOOL CxLogSystem::Stop()
{
	{
		CxCriticalSection::Owner Lock(m_csFile);
		m_File.Close();
		m_CSVFile.Close();
	}

	m_bStart = FALSE;
	return TRUE;
}

BOOL CxLogSystem::OpenLogFile( CxString& strLogPathName )
{
	if ( !m_bUseTxtFile )
		return TRUE;

	USES_CONVERSION;

	if ( !m_File.Open(strLogPathName, CxFile::modeRead|CxFile::shareDenyWrite) )
	{
		if ( !m_File.Open(strLogPathName, CxFile::modeCreate|CxFile::modeWrite) )
			return FALSE;

		CxTime CurTime = CxTime::GetCurrentTime();
		CxString strTime;
		strTime.Format( _T("%04d/%02d/%02d"), CurTime.GetYear(), CurTime.GetMonth(), CurTime.GetDay() );
		CxString strComment;
		strComment.Format( _T("################### Inspection System ##################\r\n")
						   _T("- Created by LogSystem\r\n")
						   _T("- Service Name: %s\r\n")
						   _T("- Date: %s\r\n")
						   _T("########################################################\r\n")
						   , m_strServiceName, strTime );
		m_File.Write( T2A( (LPTSTR)(LPCTSTR)strComment ), strComment.GetLength() );
	}
	m_File.Close();
	
	if ( !m_File.Open(strLogPathName, CxFile::modeWrite|CxFile::shareDenyWrite) )
		return FALSE;

	m_File.SeekToEnd();

	return TRUE;
}

BOOL CxLogSystem::OpenCSVFile( CxString& strCSVPathName )
{
	if ( !m_bUseCSVFile )
		return TRUE;
	
	if ( !m_CSVFile.Open(strCSVPathName, CxFile::modeRead|CxFile::shareDenyWrite) )
	{
		if ( !m_CSVFile.Open(strCSVPathName, CxFile::modeCreate|CxFile::modeWrite) )
			return FALSE;

		USES_CONVERSION;

		if ( !m_strCSVHeader.IsEmpty() )
			m_CSVFile.Write( T2A((LPTSTR)(LPCTSTR)m_strCSVHeader), m_strCSVHeader.GetLength() );
	}
	m_CSVFile.Close();
	
	if ( !m_CSVFile.Open(strCSVPathName, CxFile::modeWrite|CxFile::shareDenyNone) )
		return FALSE;
	
	m_CSVFile.SeekToEnd();
	
	return TRUE;
}

void CxLogSystem::LogOutA( LPCSTR lpszId, LPCSTR lpszFormat, ... )
{
	if ( lpszFormat == NULL || !m_bStart )
	{
		return;
	}

	USES_CONVERSION;
	
	va_list argList;
	va_start(argList, lpszFormat);
	
	CHAR lptszBuffer[LOG_BUFFER_SIZE];
	XVERIFY(_vsnprintf(lptszBuffer, LOG_BUFFER_SIZE, lpszFormat, argList) <= LOG_BUFFER_SIZE );

	va_end(argList);

	CxString strLog = A2T(lptszBuffer);
	CxString strId;
	if (lpszId == NULL)
		strId = _T("internal");
	else
		strId = A2T((LPSTR)lpszId);

	do 
	{
		CxCriticalSection::Owner Lock(m_csFile);

		if ( !m_File.IsOpen() ) break;

		CxTime CurTime = CxTime::GetCurrentTime();
		if ( (CurTime.GetYear() != m_Time.GetYear()) ||
			(CurTime.GetMonth() != m_Time.GetMonth()) ||
			(CurTime.GetDay() != m_Time.GetDay()) )
		{
			m_File.Close();
			m_CSVFile.Close();
			CxString strLogPathName;
			CxString strCSVPathName;
			CxString strExcelPathName;
			GetCSVPathName( CurTime, strCSVPathName );
			GetLogPathName( CurTime, strLogPathName );
			
			if ( !OpenLogFile(strLogPathName) )
			{
				break;
			}

			if ( !OpenCSVFile( strCSVPathName ) )
			{
				break;
			}

			DeleteExpiredLogFile();
			DeleteExpiredCSVFile();

			m_Time = CurTime;
		}

		if ( m_bUseCSVFile )
		{
			if ( m_CSVFile.IsOpen() )
			{
				m_strCurrentLog.Format( _T("%02d:%02d:%02d,%s,%s\r\n"), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), strId, strLog );
				m_CSVFile.Write( T2A((LPTSTR)(LPCTSTR)m_strCurrentLog), m_strCurrentLog.GetLength() );
			}
		}

		if ( m_bUseTxtFile )
		{
			m_strCurrentLog.Format( _T("%02d:%02d:%02d\t%s\t\t%s\r\n"), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), strId, strLog );
			m_File.Write( T2A((LPTSTR)(LPCTSTR)m_strCurrentLog), m_strCurrentLog.GetLength() );
		}

	} while ( FALSE );
}

void CxLogSystem::LogOutW( LPCWSTR lpszId, LPCWSTR lpszFormat, ... )
{
	if ( lpszFormat == NULL || !m_bStart )
	{
		return;
	}

	USES_CONVERSION;
	
	va_list argList;
	va_start(argList, lpszFormat);
	
	WCHAR lptszBuffer[LOG_BUFFER_SIZE];
	XVERIFY(_vsnwprintf(lptszBuffer, LOG_BUFFER_SIZE, lpszFormat, argList) <= LOG_BUFFER_SIZE );

	va_end(argList);

	CxString strLog = W2T(lptszBuffer);
	CxString strId;
	if (lpszId == NULL)
		strId = _T("internal");
	else
		strId = W2T((LPWSTR)lpszId);

	do 
	{
		CxCriticalSection::Owner Lock(m_csFile);

		if ( !m_File.IsOpen() ) break;

		CxTime CurTime = CxTime::GetCurrentTime();
		if ( (CurTime.GetYear() != m_Time.GetYear()) ||
			(CurTime.GetMonth() != m_Time.GetMonth()) ||
			(CurTime.GetDay() != m_Time.GetDay()) )
		{
			m_File.Close();
			m_CSVFile.Close();
			CxString strLogPathName;
			CxString strCSVPathName;
			CxString strExcelPathName;
			GetCSVPathName( CurTime, strCSVPathName );
			GetLogPathName( CurTime, strLogPathName );
			
			if ( !OpenLogFile(strLogPathName) )
			{
				break;
			}

			if ( !OpenCSVFile( strCSVPathName ) )
			{
				break;
			}

			DeleteExpiredLogFile();
			DeleteExpiredCSVFile();

			m_Time = CurTime;
		}

		if ( m_bUseCSVFile )
		{
			if ( m_CSVFile.IsOpen() )
			{
				m_strCurrentLog.Format( _T("%02d:%02d:%02d,%s,%s\r\n"), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), strId, strLog );
				m_CSVFile.Write( T2A((LPTSTR)(LPCTSTR)m_strCurrentLog), m_strCurrentLog.GetLength() );
			}
		}

		if ( m_bUseTxtFile )
		{
			m_strCurrentLog.Format( _T("%02d:%02d:%02d\t%s\t\t%s\r\n"), CurTime.GetHour(), CurTime.GetMinute(), CurTime.GetSecond(), strId, strLog );
			m_File.Write( T2A((LPTSTR)(LPCTSTR)m_strCurrentLog), m_strCurrentLog.GetLength() );
		}

	} while ( FALSE );
}

void CxLogSystem::DeleteExpiredCSVFile()
{
	if ( m_nExpirePeriod < 0 || !m_bUseCSVFile )
		return;
	
	CxString strFilter;
	strFilter.Format( _T("%s\\%s*.csv"), m_strLogRootPath, m_strServiceName );
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	hFind = FindFirstFile(strFilter, &FindFileData);
	
	CxString strFileName;
	CxString strDate;
	while (hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			strFileName = FindFileData.cFileName;
			strDate = FindFileData.cFileName;
			
			CxString strServiceName = strDate.ExtractLeft( _T('_') );
			
			CxString strYear = strDate.ExtractLeft( _T('_') );
			if ( strYear.IsEmpty() ) break;
			
			CxString strMonth = strDate.ExtractLeft( _T('_') );
			if ( strMonth.IsEmpty() ) break;
			
			CxString strDay = strDate.ExtractLeft( _T('.') );
			if ( strDay.IsEmpty() ) break;
			
			int nYear, nMonth, nDay;
			nYear = _tcstol( strYear, NULL, 10 );
			nMonth = _tcstol( strMonth, NULL, 10 );
			nDay = _tcstol( strDay, NULL, 10 );
			
			CxTime FindFileTime( nYear, nMonth, nDay, 0, 0, 0 );
			CxTimeSpan TimeSpan = m_Time - FindFileTime;
			if ( TimeSpan.GetDays() >= m_nExpirePeriod )
			{
				CxString strFullPathName = m_strLogRootPath;
				strFullPathName += _T('\\');
				strFullPathName += strFileName;
				DeleteFile( strFullPathName );
			}
		} while ( FALSE );
		
		if ( !FindNextFile( hFind, &FindFileData ) )
			break;
	}
	
	FindClose( hFind );
}

void CxLogSystem::DeleteExpiredLogFile()
{
	if ( m_nExpirePeriod < 0 || !m_bUseTxtFile )
		return;

	CxString strFilter;
	strFilter.Format( _T("%s\\%s*.txt"), m_strLogRootPath, m_strServiceName );
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	hFind = FindFirstFile(strFilter, &FindFileData);

	CxString strFileName;
	CxString strDate;
	while (hFind != INVALID_HANDLE_VALUE) 
	{
		do
		{
			strFileName = FindFileData.cFileName;
			strDate = FindFileData.cFileName;

			CxString strServiceName = strDate.ExtractLeft( _T('_') );

			CxString strYear = strDate.ExtractLeft( _T('_') );
			if ( strYear.IsEmpty() ) break;

			CxString strMonth = strDate.ExtractLeft( _T('_') );
			if ( strMonth.IsEmpty() ) break;

			CxString strDay = strDate.ExtractLeft( _T('.') );
			if ( strDay.IsEmpty() ) break;

			int nYear, nMonth, nDay;
			nYear = _tcstol( strYear, NULL, 10 );
			nMonth = _tcstol( strMonth, NULL, 10 );
			nDay = _tcstol( strDay, NULL, 10 );

			CxTime FindFileTime( nYear, nMonth, nDay, 0, 0, 0 );
			CxTimeSpan TimeSpan = m_Time - FindFileTime;
			if ( TimeSpan.GetDays() >= m_nExpirePeriod )
			{
				CxString strFullPathName = m_strLogRootPath;
				strFullPathName += _T('\\');
				strFullPathName += strFileName;
				DeleteFile( strFullPathName );
			}
		} while ( FALSE );
		
		if ( !FindNextFile( hFind, &FindFileData ) )
			break;
	}

	FindClose( hFind );
}