// vOleDateTime.h: interface for the CxOleDateTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VOLEDATETIME_H__3A641B41_CFC2_43A5_8427_9FFF854B67F9__INCLUDED_)
#define AFX_VOLEDATETIME_H__3A641B41_CFC2_43A5_8427_9FFF854B67F9__INCLUDED_

//#include <oleidl.h>
#include <atlbase.h>
#include <XUtil/export.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxOleDateTimeSpan;

/////////////////////////////////////////////////////////////////////////////
// CxOleCurrency class

class CxString;
class XUTIL_API CxOleCurrency
{
// Constructors
public:
	CxOleCurrency();

	CxOleCurrency(CURRENCY cySrc);
	CxOleCurrency(const CxOleCurrency& curSrc);
	CxOleCurrency(const VARIANT& varSrc);
	CxOleCurrency(long nUnits, long nFractionalUnits);

// Attributes
public:
	enum CurrencyStatus
	{
		valid = 0,
		invalid = 1,    // Invalid currency (overflow, div 0, etc.)
		null = 2,       // Literally has no value
	};

	CURRENCY m_cur;
	CurrencyStatus m_status;

	void SetStatus(CurrencyStatus status);
	CurrencyStatus GetStatus() const;

// Operations
public:
	const CxOleCurrency& operator=(CURRENCY cySrc);
	const CxOleCurrency& operator=(const CxOleCurrency& curSrc);
	const CxOleCurrency& operator=(const VARIANT& varSrc);

	BOOL operator==(const CxOleCurrency& cur) const;
	BOOL operator!=(const CxOleCurrency& cur) const;
	BOOL operator<(const CxOleCurrency& cur) const;
	BOOL operator>(const CxOleCurrency& cur) const;
	BOOL operator<=(const CxOleCurrency& cur) const;
	BOOL operator>=(const CxOleCurrency& cur) const;

	// Currency math
	CxOleCurrency operator+(const CxOleCurrency& cur) const;
	CxOleCurrency operator-(const CxOleCurrency& cur) const;
	const CxOleCurrency& operator+=(const CxOleCurrency& cur);
	const CxOleCurrency& operator-=(const CxOleCurrency& cur);
	CxOleCurrency operator-() const;

	CxOleCurrency operator*(long nOperand) const;
	CxOleCurrency operator/(long nOperand) const;
	const CxOleCurrency& operator*=(long nOperand);
	const CxOleCurrency& operator/=(long nOperand);

	operator CURRENCY() const;

	// Currency definition
	void SetCurrency(long nUnits, long nFractionalUnits);
	BOOL ParseCurrency(LPCTSTR lpszCurrency, DWORD dwFlags = 0,
		LCID = LANG_USER_DEFAULT);

	// formatting
	CxString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const;
};

class XUTIL_API CxOleDateTime
{
// Constructors
public:
	static CxOleDateTime PASCAL GetCurrentTime();

	CxOleDateTime();

	CxOleDateTime(const CxOleDateTime& dateSrc);
	CxOleDateTime(const VARIANT& varSrc);
	CxOleDateTime(DATE dtSrc);

	CxOleDateTime(time_t timeSrc);
	CxOleDateTime(const SYSTEMTIME& systimeSrc);
	CxOleDateTime(const FILETIME& filetimeSrc);

	CxOleDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec);

// Attributes
public:
	enum DateTimeStatus
	{
		valid = 0,
		invalid = 1,    // Invalid date (out of range, etc.)
		null = 2,       // Literally has no value
	};

	DATE m_dt;
	DateTimeStatus m_status;

	void SetStatus(DateTimeStatus status);
	DateTimeStatus GetStatus() const;

	BOOL GetAsSystemTime(SYSTEMTIME& sysTime) const;

	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month (0-31)
	int GetHour() const;        // hour in day (0-23)
	int GetMinute() const;      // minute in hour (0-59)
	int GetSecond() const;      // second in minute (0-59)
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat
	int GetDayOfYear() const;   // days since start of year, Jan 1 = 1

// Operations
public:
	const CxOleDateTime& operator=(const CxOleDateTime& dateSrc);
	const CxOleDateTime& operator=(const VARIANT& varSrc);
	const CxOleDateTime& operator=(DATE dtSrc);

	const CxOleDateTime& operator=(const time_t& timeSrc);
	const CxOleDateTime& operator=(const SYSTEMTIME& systimeSrc);
	const CxOleDateTime& operator=(const FILETIME& filetimeSrc);

	BOOL operator==(const CxOleDateTime& date) const;
	BOOL operator!=(const CxOleDateTime& date) const;
	BOOL operator<(const CxOleDateTime& date) const;
	BOOL operator>(const CxOleDateTime& date) const;
	BOOL operator<=(const CxOleDateTime& date) const;
	BOOL operator>=(const CxOleDateTime& date) const;

	// DateTime math
	CxOleDateTime operator+(const CxOleDateTimeSpan& dateSpan) const;
	CxOleDateTime operator-(const CxOleDateTimeSpan& dateSpan) const;
	const CxOleDateTime& operator+=(const CxOleDateTimeSpan dateSpan);
	const CxOleDateTime& operator-=(const CxOleDateTimeSpan dateSpan);

	// DateTimeSpan math
	CxOleDateTimeSpan operator-(const CxOleDateTime& date) const;

	operator DATE() const;

	int SetDateTime(int nYear, int nMonth, int nDay,
		int nHour, int nMin, int nSec);
	int SetDate(int nYear, int nMonth, int nDay);
	int SetTime(int nHour, int nMin, int nSec);
	BOOL ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags = 0,
		LCID lcid = LANG_USER_DEFAULT);

	// formatting
	CxString Format(DWORD dwFlags = 0, LCID lcid = LANG_USER_DEFAULT) const;
	CxString Format(LPCTSTR lpszFormat) const;

// Implementation
protected:
	void CheckRange();
	friend CxOleDateTimeSpan;
};

/////////////////////////////////////////////////////////////////////////////
// CxOleDateTimeSpan class
class XUTIL_API CxOleDateTimeSpan
{
// Constructors
public:
	CxOleDateTimeSpan();

	CxOleDateTimeSpan(double dblSpanSrc);
	CxOleDateTimeSpan(const CxOleDateTimeSpan& dateSpanSrc);
	CxOleDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

// Attributes
public:
	enum DateTimeSpanStatus
	{
		valid = 0,
		invalid = 1,    // Invalid span (out of range, etc.)
		null = 2,       // Literally has no value
	};

	double m_span;
	DateTimeSpanStatus m_status;

	void SetStatus(DateTimeSpanStatus status);
	DateTimeSpanStatus GetStatus() const;

	double GetTotalDays() const;    // span in days (about -3.65e6 to 3.65e6)
	double GetTotalHours() const;   // span in hours (about -8.77e7 to 8.77e6)
	double GetTotalMinutes() const; // span in minutes (about -5.26e9 to 5.26e9)
	double GetTotalSeconds() const; // span in seconds (about -3.16e11 to 3.16e11)

	long GetDays() const;       // component days in span
	long GetHours() const;      // component hours in span (-23 to 23)
	long GetMinutes() const;    // component minutes in span (-59 to 59)
	long GetSeconds() const;    // component seconds in span (-59 to 59)

// Operations
public:
	const CxOleDateTimeSpan& operator=(double dblSpanSrc);
	const CxOleDateTimeSpan& operator=(const CxOleDateTimeSpan& dateSpanSrc);

	BOOL operator==(const CxOleDateTimeSpan& dateSpan) const;
	BOOL operator!=(const CxOleDateTimeSpan& dateSpan) const;
	BOOL operator<(const CxOleDateTimeSpan& dateSpan) const;
	BOOL operator>(const CxOleDateTimeSpan& dateSpan) const;
	BOOL operator<=(const CxOleDateTimeSpan& dateSpan) const;
	BOOL operator>=(const CxOleDateTimeSpan& dateSpan) const;

	// DateTimeSpan math
	CxOleDateTimeSpan operator+(const CxOleDateTimeSpan& dateSpan) const;
	CxOleDateTimeSpan operator-(const CxOleDateTimeSpan& dateSpan) const;
	const CxOleDateTimeSpan& operator+=(const CxOleDateTimeSpan dateSpan);
	const CxOleDateTimeSpan& operator-=(const CxOleDateTimeSpan dateSpan);
	CxOleDateTimeSpan operator-() const;

	operator double() const;

	void SetDateTimeSpan(long lDays, int nHours, int nMins, int nSecs);

	// formatting
	CxString Format(LPCTSTR pFormat) const;

// Implementation
public:
	void CheckRange();
	friend CxOleDateTime;
};

typedef const VARIANT* LPCVARIANT;

class XUTIL_API CxOleVariant : public tagVARIANT
{
// Constructors
public:
	CxOleVariant();

	CxOleVariant(const VARIANT& varSrc);
	CxOleVariant(LPCVARIANT pSrc);
	CxOleVariant(const CxOleVariant& varSrc);

	CxOleVariant(LPCTSTR lpszSrc);
	CxOleVariant(LPCTSTR lpszSrc, VARTYPE vtSrc); // used to set to ANSI string
	CxOleVariant(CxString& strSrc);

	CxOleVariant(BYTE nSrc);
	CxOleVariant(short nSrc, VARTYPE vtSrc = VT_I2);
	CxOleVariant(long lSrc, VARTYPE vtSrc = VT_I4);
	CxOleVariant(const CxOleCurrency& curSrc);

	CxOleVariant(float fltSrc);
	CxOleVariant(double dblSrc);
	CxOleVariant(const CxOleDateTime& timeSrc);

// Operations
public:
	void Clear();
	void ChangeType(VARTYPE vartype, LPVARIANT pSrc = NULL);
	void Attach(VARIANT& varSrc);
	VARIANT Detach();

	BOOL operator==(const VARIANT& varSrc) const;
	BOOL operator==(LPCVARIANT pSrc) const;

	const CxOleVariant& operator=(const VARIANT& varSrc);
	const CxOleVariant& operator=(LPCVARIANT pSrc);
	const CxOleVariant& operator=(const CxOleVariant& varSrc);

	const CxOleVariant& operator=(const LPCTSTR lpszSrc);
	const CxOleVariant& operator=(const CxString& strSrc);

	const CxOleVariant& operator=(BYTE nSrc);
	const CxOleVariant& operator=(short nSrc);
	const CxOleVariant& operator=(long lSrc);
	const CxOleVariant& operator=(const CxOleCurrency& curSrc);

	const CxOleVariant& operator=(float fltSrc);
	const CxOleVariant& operator=(double dblSrc);
	const CxOleVariant& operator=(const CxOleDateTime& dateSrc);

	void SetString(LPCTSTR lpszSrc, VARTYPE vtSrc); // used to set ANSI string

	operator LPVARIANT();
	operator LPCVARIANT() const;

// Implementation
public:
	~CxOleVariant();
	void _ClearCompat();
};

#define _OLE_DATETIME_HALFSECOND (1.0 / (2.0 * (60.0 * 60.0 * 24.0)))


#endif // !defined(AFX_VOLEDATETIME_H__3A641B41_CFC2_43A5_8427_9FFF854B67F9__INCLUDED_)
