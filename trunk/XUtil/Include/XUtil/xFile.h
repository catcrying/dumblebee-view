#ifndef __X_FILE_H__
#define __X_FILE_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/export.h>
#include <XUtil/xArchive.h>
#include <vector>

class CxString;
class XUTIL_API CxFile : public CxArchive
{

public:
	enum SeekPosition
	{
		begin = FILE_BEGIN,
		current = FILE_CURRENT,
		end = FILE_END
	};

	enum OpenFlags
	{
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		modeNoTruncate =    0x2000,
		typeText =          0x4000,
		typeBinary =   (int)0x8000
	};

	BOOL m_bLoading;

	CxFile();
	CxFile(LPCTSTR pStrFileName);

	CxFile(const CxFile& op);
	CxFile(HANDLE hFile);

	CxFile(LPCTSTR pStrFileName, unsigned int nOpenFlags);

	virtual ~CxFile();
	virtual BOOL IsLoading();
	virtual BOOL IsStoring();
	BOOL IsOpen();
	virtual DWORD GetPosition();
	virtual BOOL Flush();
	virtual unsigned int Read(void* lpBuf, unsigned int nMax);
	virtual unsigned int Write(const void* lpData, unsigned int nSize);

	virtual BOOL Open(LPCTSTR lpszFileName, unsigned int nOpenFlags);

	virtual void Close();
	virtual long Seek(long lOff, unsigned int nFrom);
	virtual BOOL SetLength(DWORD dwNewLen);
	virtual DWORD GetLength();
	DWORD SeekToEnd();
	void SeekToBegin();

protected:
	HANDLE m_hFile;
	DWORD  m_dwAccess;
	BOOL   m_bCloseOnDelete;
};

#endif // __X_FILE_H__
