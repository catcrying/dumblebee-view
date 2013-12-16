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
class XUTIL_API CxTimeSpan
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

inline CxTimeSpan::CxTimeSpan() { }
inline CxTimeSpan::CxTimeSpan(time_t time) { m_timeSpan = time; }
inline CxTimeSpan::CxTimeSpan(LONG lDays, int nHours, int nMins, int nSecs) { m_timeSpan = nSecs + 60* (nMins + 60* (nHours + 24* lDays)); }
inline CxTimeSpan::CxTimeSpan(const CxTimeSpan& timeSpanSrc) { m_timeSpan = timeSpanSrc.m_timeSpan; }
inline const CxTimeSpan& CxTimeSpan::operator=(const CxTimeSpan& timeSpanSrc)	{ m_timeSpan = timeSpanSrc.m_timeSpan; return *this; }
inline LONG CxTimeSpan::GetDays() const	{ return (LONG)(m_timeSpan / (24*3600L)); }
inline LONG CxTimeSpan::GetTotalHours() const { return (LONG)(m_timeSpan/3600); }
inline int CxTimeSpan::GetHours() const	{ return (int)(GetTotalHours() - GetDays()*24); }
inline LONG CxTimeSpan::GetTotalMinutes() const	{ return (LONG)(m_timeSpan/60); }
inline int CxTimeSpan::GetMinutes() const { return (int)(GetTotalMinutes() - GetTotalHours()*60); }
inline LONG CxTimeSpan::GetTotalSeconds() const	{ return (LONG)m_timeSpan; }
inline int CxTimeSpan::GetSeconds() const { return (int)(GetTotalSeconds() - GetTotalMinutes()*60); }
inline CxTimeSpan CxTimeSpan::operator-(CxTimeSpan timeSpan) const { return CxTimeSpan(m_timeSpan - timeSpan.m_timeSpan); }
inline CxTimeSpan CxTimeSpan::operator+(CxTimeSpan timeSpan) const { return CxTimeSpan(m_timeSpan + timeSpan.m_timeSpan); }
inline const CxTimeSpan& CxTimeSpan::operator+=(CxTimeSpan timeSpan) { m_timeSpan += timeSpan.m_timeSpan; return *this; }
inline const CxTimeSpan& CxTimeSpan::operator-=(CxTimeSpan timeSpan) { m_timeSpan -= timeSpan.m_timeSpan; return *this; }
inline BOOL CxTimeSpan::operator==(CxTimeSpan timeSpan) const { return m_timeSpan == timeSpan.m_timeSpan; }
inline BOOL CxTimeSpan::operator!=(CxTimeSpan timeSpan) const { return m_timeSpan != timeSpan.m_timeSpan; }
inline BOOL CxTimeSpan::operator<(CxTimeSpan timeSpan) const { return m_timeSpan < timeSpan.m_timeSpan; }
inline BOOL CxTimeSpan::operator>(CxTimeSpan timeSpan) const	{ return m_timeSpan > timeSpan.m_timeSpan; }
inline BOOL CxTimeSpan::operator<=(CxTimeSpan timeSpan) const { return m_timeSpan <= timeSpan.m_timeSpan; }
inline BOOL CxTimeSpan::operator>=(CxTimeSpan timeSpan) const { return m_timeSpan >= timeSpan.m_timeSpan; }

inline CxTime::CxTime() { }
inline CxTime::CxTime(time_t time) { m_time = time; }
inline CxTime::CxTime(const CxTime& timeSrc) { m_time = timeSrc.m_time; }
inline const CxTime& CxTime::operator=(const CxTime& timeSrc) { m_time = timeSrc.m_time; return *this; }
inline const CxTime& CxTime::operator=(time_t t)	{ m_time = t; return *this; }
inline time_t CxTime::GetTime() const { return m_time; }
inline int CxTime::GetYear() const { return (GetLocalTm(NULL)->tm_year) + 1900; }
inline int CxTime::GetMonth() const	{ return GetLocalTm(NULL)->tm_mon + 1; }
inline int CxTime::GetDay() const { return GetLocalTm(NULL)->tm_mday; }
inline int CxTime::GetHour() const { return GetLocalTm(NULL)->tm_hour; }
inline int CxTime::GetMinute() const { return GetLocalTm(NULL)->tm_min; }
inline int CxTime::GetSecond() const { return GetLocalTm(NULL)->tm_sec; }
inline int CxTime::GetDayOfWeek() const { return GetLocalTm(NULL)->tm_wday + 1; }
inline CxTimeSpan CxTime::operator-(CxTime time) const { return CxTimeSpan(m_time - time.m_time); }
inline CxTime CxTime::operator-(CxTimeSpan timeSpan) const { return CxTime(m_time - timeSpan.m_timeSpan); }
inline CxTime CxTime::operator+(CxTimeSpan timeSpan) const { return CxTime(m_time + timeSpan.m_timeSpan); }
inline const CxTime& CxTime::operator+=(CxTimeSpan timeSpan) { m_time += timeSpan.m_timeSpan; return *this; }
inline const CxTime& CxTime::operator-=(CxTimeSpan timeSpan) { m_time -= timeSpan.m_timeSpan; return *this; }
inline BOOL CxTime::operator==(CxTime time) const { return m_time == time.m_time; }
inline BOOL CxTime::operator!=(CxTime time) const { return m_time != time.m_time; }
inline BOOL CxTime::operator<(CxTime time) const { return m_time < time.m_time; }
inline BOOL CxTime::operator>(CxTime time) const { return m_time > time.m_time; }
inline BOOL CxTime::operator<=(CxTime time) const { return m_time <= time.m_time; }
inline BOOL CxTime::operator>=(CxTime time) const { return m_time >= time.m_time; }

#include <XUtil/xTime.inl>

#endif // __X_TIME_H__