#include "stdafx.h"

#include <XUtil/xUtils.h>
#include <XUtil/xException.h>
#include <XUtil/xCriticalSection.h>

#include <memory>

#define UNLEN 256 

using std::auto_ptr;

void MakeDirectory( CxString strDir )
{
	if ( strDir.Right(1) == _T("\\") )
		strDir = strDir.Left(strDir.GetLength()-1); 
	if ( GetFileAttributes(strDir) != -1 )
		return;

	DWORD dwErr = GetLastError();
	if ( !((dwErr == ERROR_PATH_NOT_FOUND) || (dwErr == ERROR_FILE_NOT_FOUND)) )
		return;

	int nFound = strDir.ReverseFind( _T('\\') );
	MakeDirectory( strDir.Left(nFound) );
	::CreateDirectory( strDir, NULL );
}

CxString GetLastErrorMessage(DWORD dwLastError)
{
	static TCHAR errmsg[512];

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
		0,
		dwLastError,
		0,
		errmsg, 
		511,
		NULL))
	{
		/* if we fail, call ourself to find out why and return that error */
		return (GetLastErrorMessage(GetLastError()));  
	}

	return errmsg;
}

BOOL IsExistFile( CxString strPathName )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = ::FindFirstFile( strPathName, &FindFileData );
	if ( hFind == INVALID_HANDLE_VALUE )
	{
		::FindClose( hFind );
		return FALSE;
	}
	else
	{
		::FindClose( hFind );
		return TRUE;
	}
}

void DeleteDirectory( CxString strDirectory )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	BOOL bWorking = TRUE;
	CxString strDirFile = strDirectory + CxString("\\*.*");
	hFind = ::FindFirstFile( strDirFile, &FindFileData );
	while ( hFind != INVALID_HANDLE_VALUE )
	{
		if ( (FindFileData.cFileName[0] == _T('.') && FindFileData.cFileName[1] == _T('\0')) ||
			(FindFileData.cFileName[0] == _T('.') && FindFileData.cFileName[1] == _T('.')  && FindFileData.cFileName[2] == _T('\0')) )
		{
			if ( !::FindNextFile( hFind, &FindFileData ) )
				break;
			continue;
		}

		CxString strPathName;
		strPathName.Format( _T("%s\\%s"), strDirectory, FindFileData.cFileName );

		if ( !!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			DeleteDirectory( strPathName );
		}
		else
		{
			::DeleteFile( strPathName );
		}
		if ( !::FindNextFile( hFind, &FindFileData ) )
			break;
	}
	::FindClose( hFind );
	::RemoveDirectory( strDirectory );
}


CxString HexToString( const BYTE *pBuffer, size_t iBytes )
{
	CxString result;

	for (size_t i = 0; i < iBytes; i++)
	{
		BYTE c ;

		BYTE b = pBuffer[i] >> 4;

		if (9 >= b)
		{
			c = b + '0';
		}
		else
		{
			c = (b - 10) + 'A';
		}

		result += (TCHAR)c;

		b = pBuffer[i] & 0x0f;

		if (9 >= b)
		{
			c = b + '0';
		}
		else
		{
			c = (b - 10) + 'A';
		}

		result += (TCHAR)c;
	}

	return result;
}

void StringToHex( const CxString &ts, BYTE *pBuffer, size_t nBytes )
{
	USES_CONVERSION;

	LPSTR s = T2A(const_cast<PTSTR>((LPCTSTR)ts));

	for (size_t i = 0; i < nBytes; i++)
	{
		const size_t stringOffset = i * 2;

		BYTE val = 0;

		const BYTE b = s[stringOffset];

		if (isdigit(b)) 
		{
			val = (BYTE)((b - '0') * 16); 
		}
		else 
		{
			val = (BYTE)(((toupper(b) - 'A') + 10) * 16); 
		}

		const BYTE b1 = s[stringOffset + 1];

		if (isdigit(b1)) 
		{
			val += b1 - '0' ; 
		}
		else 
		{
			val += (BYTE)((toupper(b1) - 'A') + 10); 
		}

		pBuffer[i] = val;
	}
}

CxString GetCurrentDirectory()
{
	DWORD size = ::GetCurrentDirectory(0, 0);

	auto_ptr<TCHAR> spBuf(new TCHAR[size]);

	if (0 == ::GetCurrentDirectory(size, spBuf.get()))
	{
		throw CxException(_T("GetCurrentDirectory()"), _T("Failed to get current directory"));
	}

	return CxString(spBuf.get());
}

CxString GetExecuteDirectory()
{
#ifndef MAX_PATH
#	define MAX_PATH 260
#endif
	TCHAR spBuf[MAX_PATH];
	::GetModuleFileName(NULL, spBuf, sizeof(spBuf));

	CxString strFullPath( spBuf ); 

	int nBackSlashPos = strFullPath.ReverseFind( _T('\\') );

	CxString strPath = strFullPath.Mid( 0, nBackSlashPos );

	return strPath;
}

CxString RelativePathToAbsolutePath( CxString strRelativePath )
{
	if ( strRelativePath.GetAt(0) == _T('\\') || strRelativePath.GetAt(0) == _T('/') )
	{
		return strRelativePath;
	}
	if ( strRelativePath.Find(_T(':'), 0) > 0 )	return strRelativePath;

	CxString strExePath = GetExecuteDirectory();

	CxString strAbsolutePathName;
	strAbsolutePathName.MakePathName(strExePath, strRelativePath);

	return strAbsolutePathName;
}

CxString GetExecuteFile()
{
	TCHAR    chPath[_MAX_PATH];
	TCHAR    chDrive[_MAX_DRIVE];
	TCHAR    chDir[_MAX_DIR];
	TCHAR    chFName[_MAX_FNAME];
	TCHAR    chExt[_MAX_EXT];
	CxString strFile;

	::GetModuleFileName(NULL, chPath, MAX_PATH);
	_tsplitpath(chPath, chDrive, chDir, chFName, chExt);
	strFile.Format(_T("%s"), chFName);
	strFile.MakeUpper();

	return strFile;
}

CxString GetDateStamp()
{
	SYSTEMTIME systime;
	::GetSystemTime(&systime);

	static TCHAR buffer[7];

	_stprintf(buffer, _T("%02d/%02d/%02d"),
		( 1900 + systime.wYear) % 100,
		systime.wMonth,
		systime.wDay );

	return CxString(buffer);
}

CxString GetTime()
{
	SYSTEMTIME systime;
	::GetLocalTime(&systime);

	static TCHAR buffer[7];

	/*_stprintf*/wsprintf(buffer, _T("%02d:%02d:%02d"),
		systime.wHour,
		systime.wMinute,
		systime.wSecond );

	return CxString(buffer);
}

CxString ToHexC(BYTE c)
{
	TCHAR hex[3];

	const int val = c;

	/*_stprintf*/wsprintf(hex, _T("%02X"), val);

	return CxString(hex);
}

CxString DumpData(const BYTE * const pData, size_t dataLength, size_t lineLength /* = 0 */)
{
	const size_t bytesPerLine = lineLength != 0 ? (lineLength - 1) / 3 : 0;

	CxString result;

	CxString hexDisplay;
	CxString display;

	size_t i = 0;

	while (i < dataLength)
	{
		const BYTE c = pData[i++];

		hexDisplay += ToHexC(c) + _T(" ");

		if (isprint(c))
		{
			display += (TCHAR)c;
		}
		else
		{
			display += _T('.');
		}

		if ((bytesPerLine && (i % bytesPerLine == 0 && i != 0)) || i == dataLength)
		{
			if (i == dataLength && (bytesPerLine && (i % bytesPerLine != 0)))
			{
				for (size_t pad = i % bytesPerLine; pad < bytesPerLine; pad++)
				{
					hexDisplay += _T("   ");
				}
			}
			result += hexDisplay + _T(" - ") + display + _T("\n");

			hexDisplay = _T("");
			display = _T("");
		}
	}

	return result;
}

CxString GetComputerName()
{
	static BOOL bGotName = FALSE;

	static CxString strName = _T("UNAVAILABLE");

	if (!bGotName)
	{
		TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1] ;
		DWORD computerNameLen = MAX_COMPUTERNAME_LENGTH ;

		if (::GetComputerName(computerName, &computerNameLen))
		{
			strName = computerName;
		}

		bGotName = TRUE;
	}

	return strName;
}

CxString GetModuleFileName( HINSTANCE hModule /* = NULL */ )
{
	static BOOL bGotName = FALSE;

	static CxString strName = _T("UNAVAILABLE");

	if (!bGotName)
	{
		TCHAR moduleFileName[MAX_PATH + 1] ;
		DWORD moduleFileNameLen = MAX_PATH ;

		if (::GetModuleFileName(hModule, moduleFileName, moduleFileNameLen))
		{
			strName = moduleFileName;
		}

		bGotName = TRUE;
	}

	return strName;
}

CxString GetUserName()
{
	static bool bGotName = FALSE;

	static CxString strName = _T("UNAVAILABLE");

	if (!bGotName)
	{
		TCHAR userName[UNLEN + 1] ;
		DWORD userNameLen = UNLEN;

		if (::GetUserName(userName, &userNameLen))
		{
			strName = userName;
		}

		bGotName = TRUE;
	}

	return strName;
}

CxString StripLeading( const CxString &source, const char toStrip )
{
	const TCHAR *pSrc = (LPCTSTR)source;

	while (pSrc && *pSrc == toStrip)
	{
		++pSrc;
	}

	return pSrc;
}

CxString StripTrailing( const CxString &source, const char toStrip )
{
	size_t i = source.GetLength();
	const _TCHAR *pSrc = (LPCTSTR)source + i;

	--pSrc;

	while (i && *pSrc == toStrip)
	{
		--pSrc;
		--i;
	}

	return source.Mid(0, (int)i);
}

CxString GetFileVersion()
{
	CxString version;

	const CxString moduleFileName = GetModuleFileName(NULL);

	LPTSTR pModuleFileName = const_cast<LPTSTR>((LPCTSTR)moduleFileName);

	DWORD zero = 0;

	DWORD verSize = ::GetFileVersionInfoSize(pModuleFileName, &zero);

	if (verSize != 0)
	{
		auto_ptr<BYTE> spBuffer(new BYTE[verSize]);

		if (::GetFileVersionInfo(pModuleFileName, 0, verSize, spBuffer.get()))
		{
			LPTSTR pVersion = 0;
			UINT verLen = 0;

			if (::VerQueryValue(spBuffer.get(), 
				const_cast<LPTSTR>(_T("\\StringFileInfo\\080904b0\\ProductVersion")), 
				(void**)&pVersion, 
				&verLen))
			{
				version = pVersion;
			}
		}
	}

	return version;
}
