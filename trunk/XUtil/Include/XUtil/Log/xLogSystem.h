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

	void UseCSVFileA( BOOL bUse, LPCSTR lpszCSVHeader );
	void UseCSVFileW( BOOL bUse, LPCWSTR lpszCSVHeader );
#ifdef _UNICODE
	#define UseCSVFile	UseCSVFileW
#else
	#define UseCSVFile	UseCSVFileA
#endif

	void UseMonthlySplitFolder( BOOL bUse );

	void SetLogDirectoryA( LPCSTR lpszPath );
	void SetLogDirectoryW( LPCWSTR lpszPath );
#ifdef _UNICODE
	#define SetLogDirectory	SetLogDirectoryW
#else
	#define SetLogDirectory	SetLogDirectoryA
#endif

	void SetExpirePeriod( int nDay ) { m_nExpirePeriod = nDay; }
	
	void LogOutA( LPCSTR lpszId, LPCSTR lpszFormat, ... );
	void LogOutW( LPCWSTR lpszId, LPCWSTR lpszFormat, ... );
#ifdef _UNICODE
	#define LogOut	LogOutW
#else
	#define LogOut	LogOutA
#endif

	BOOL Start();
	BOOL Stop();

};

#endif // __X_LOGSYSTEM_H__