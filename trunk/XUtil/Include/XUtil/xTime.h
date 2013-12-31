#ifndef __X_TIME_H__
#define __X_TIME_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <time.h>

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <XUtil/String/xString.h>

/////////////////////////////////////////////////////////////////////////////
// CxTimeSpan and CxTime
class CxTime;
class CxTimeSpan
{
public:

// Constructors
	CxTimeSpan();
	CxTimeSpan(time_t time);
	CxTimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

	CxTimeSpan(const CxTimeSpan& timeSpanSrc);
	const CxTimeSpan& operator=(const CxTimeSpan& timeSpanSrc);

// Attributes
	// extract parts
	LONG GetDays() const;   // total # of days
	LONG GetTotalHours() const;
	int GetHours() const;
	LONG GetTotalMinutes() const;
	int GetMinutes() const;
	LONG GetTotalSeconds() const;
	int GetSeconds() const;

// Operations
	// time math
	CxTimeSpan operator-(CxTimeSpan timeSpan) const;
	CxTimeSpan operator+(CxTimeSpan timeSpan) const;
	const CxTimeSpan& operator+=(CxTimeSpan timeSpan);
	const CxTimeSpan& operator-=(CxTimeSpan timeSpan);
	BOOL operator==(CxTimeSpan timeSpan) const;
	BOOL operator!=(CxTimeSpan timeSpan) const;
	BOOL operator<(CxTimeSpan timeSpan) const;
	BOOL operator>(CxTimeSpan timeSpan) const;
	BOOL operator<=(CxTimeSpan timeSpan) const;
	BOOL operator>=(CxTimeSpan timeSpan) const;

	CxString Format(LPCTSTR pFormat) const;

private:
	time_t m_timeSpan;
	friend class CxTime;
};

class XUTIL_API CxTime
{
public:

// Constructors
	static CxTime PASCAL GetCurrentTime();

	CxTime();
	CxTime(time_t time);
	CxTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1);
	CxTime(WORD wDosDate, WORD wDosTime, int nDST = -1);
	CxTime(const CxTime& timeSrc);

	CxTime(const SYSTEMTIME& sysTime, int nDST = -1);
	CxTime(const FILETIME& fileTime, int nDST = -1);
	const CxTime& operator=(const CxTime& timeSrc);
	const CxTime& operator=(time_t t);

// Attributes
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;
	BOOL GetAsSystemTime(SYSTEMTIME& timeDest) const;

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;       // month of year (1 = Jan)
	int GetDay() const;         // day of month
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;   // 1=Sun, 2=Mon, ..., 7=Sat

// Operations
	// time math
	CxTimeSpan operator-(CxTime time) const;
	CxTime operator-(CxTimeSpan timeSpan) const;
	CxTime operator+(CxTimeSpan timeSpan) const;
	const CxTime& operator+=(CxTimeSpan timeSpan);
	const CxTime& operator-=(CxTimeSpan timeSpan);
	BOOL operator==(CxTime time) const;
	BOOL operator!=(CxTime time) const;
	BOOL operator<(CxTime time) const;
	BOOL operator>(CxTime time) const;
	BOOL operator<=(CxTime time) const;
	BOOL operator>=(CxTime time) const;

	// formatting using "C" strftime
	CxString Format(LPCTSTR pFormat) const;
	CxString FormatGmt(LPCTSTR pFormat) const;

private:
	time_t m_time;
};

#include <XUtil/xTime.inl>

#endif // __X_TIME_H__