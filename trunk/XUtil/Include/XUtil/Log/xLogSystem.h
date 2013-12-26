#ifndef __X_LOGSYSTEM_H__
#define __X_LOGSYSTEM_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xFile.h>
#include <XUtil/String/xString.h>
#include <XUtil/xCriticalSection.h>
#include <XUtil/xTime.h>
#include <XUtil/xAutoResetEvent.h>
#include <XUtil/xManualResetEvent.h>

#include <vector>


#define LOG_BUFFER_SIZE		10240

class XUTIL_API CxLogSystem  
{
private:
	CxFile		m_File;
	CxFile		m_CSVFile;
	CxString	m_strLogRootPath;
	CxString	m_strServiceName;
	int			m_nExpirePeriod;
	CxString	m_strCurrentLog;
	CxTime		m_Time;

	CxString	m_strCSVHeader;

	CxCriticalSection   m_csFile;

	BOOL		m_bUseCSVFile;
	BOOL		m_bUseTxtFile;

	BOOL		m_bMonthlySplitFolder;
	
	BOOL		m_bStart;

protected:
	BOOL OpenLogFile( CxString& strLogPathName );
	void DeleteExpiredLogFile();
	void DeleteExpiredCSVFile();

	BOOL OpenCSVFile( CxString& strCSVPathName );
	void GetCSVPathName( CxTime& Time, CxString& strCSVPathName );
	void GetLogPathName( CxTime& Time, CxString& strLogPathName );

public:
	explicit CxLogSystem( LPCTSTR lpszServiceName );
	virtual ~CxLogSystem();

	void UseTextFile( BOOL bUse );
	void UseCSVFile( BOOL bUse, LPCTSTR lpszCSVHeader );

	void UseMonthlySplitFolder( BOOL bUse );

	void SetLogDirectory( LPCTSTR lpszPath );

	void SetExpirePeriod( int nDay ) { m_nExpirePeriod = nDay; }
	
	void LogOut( LPCTSTR lpszId, LPCTSTR lpszFormat, ... );

	BOOL Start();
	BOOL Stop();

};

#endif // __X_LOGSYSTEM_H__