// xTime.inl: implementation of the CxTime class.
//
//////////////////////////////////////////////////////////////////////

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

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif

inline CxTime::CxTime( int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST )
{
	struct tm atm;
	atm.tm_sec = nSec;
	atm.tm_min = nMin;
	atm.tm_hour = nHour;
	XASSERT(nDay >= 1 && nDay <= 31);
	atm.tm_mday = nDay;
	XASSERT(nMonth >= 1 && nMonth <= 12);
	atm.tm_mon = nMonth - 1;        // tm_mon is 0 based
	XASSERT(nYear >= 1900);
	atm.tm_year = nYear - 1900;     // tm_year is 1900 based
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	XASSERT(m_time != -1);       // indicates an illegal input time
}

inline CxTime::CxTime( WORD wDosDate, WORD wDosTime, int nDST )
{
	struct tm atm;
	atm.tm_sec = (wDosTime & ~0xFFE0) << 1;
	atm.tm_min = (wDosTime & ~0xF800) >> 5;
	atm.tm_hour = wDosTime >> 11;

	atm.tm_mday = wDosDate & ~0xFFE0;
	atm.tm_mon = ((wDosDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (wDosDate >> 9) + 80;
	atm.tm_isdst = nDST;
	m_time = mktime(&atm);
	XASSERT(m_time != -1);       // indicates an illegal input time
}

inline CxTime::CxTime( const SYSTEMTIME& sysTime, int nDST )
{
	if (sysTime.wYear < 1900)
	{
		time_t time0 = 0L;
		CxTime timeT(time0);
		*this = timeT;
	}
	else
	{
		CxTime timeT(
			(int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay,
			(int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond,
			nDST);
		*this = timeT;
	}
}

inline CxTime::CxTime(const FILETIME& fileTime, int nDST)
{
	// first convert file time (UTC time) to local time
	FILETIME localTime;
	if (!FileTimeToLocalFileTime(&fileTime, &localTime))
	{
		m_time = 0;
		return;
	}

	// then convert that time to system time
	SYSTEMTIME sysTime;
	if (!FileTimeToSystemTime(&localTime, &sysTime))
	{
		m_time = 0;
		return;
	}

	// then convert the system time to a time_t (C-runtime local time)
	CxTime timeT(sysTime, nDST);
	*this = timeT;
}

inline CxTime PASCAL CxTime::GetCurrentTime()
// return the current system time
{
	return CxTime(::time(NULL));
}

inline struct tm* CxTime::GetGmtTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		gmtime_s(ptm, &m_time);
		return ptm;
	}
	else
	{
		static struct tm _tm;
		gmtime_s(&_tm, &m_time);
		return &_tm;
	}
}

inline struct tm* CxTime::GetLocalTm(struct tm* ptm) const
{
	errno_t err;
	if (ptm != NULL)
	{
		err = localtime_s(ptm, &m_time);
		if (err)
			return NULL;    // indicates the m_time was not initialized!

		return ptm;
	}
	else
	{
		static struct tm _tm;
		err = localtime_s(&_tm, &m_time);
		if (err)
			return NULL;
		return &_tm;
	}
}

inline BOOL CxTime::GetAsSystemTime(SYSTEMTIME& timeDest) const
{
	struct tm* ptm = GetLocalTm(NULL);
	if (ptm == NULL)
		return FALSE;

	timeDest.wYear = (WORD) (1900 + ptm->tm_year);
	timeDest.wMonth = (WORD) (1 + ptm->tm_mon);
	timeDest.wDayOfWeek = (WORD) ptm->tm_wday;
	timeDest.wDay = (WORD) ptm->tm_mday;
	timeDest.wHour = (WORD) ptm->tm_hour;
	timeDest.wMinute = (WORD) ptm->tm_min;
	timeDest.wSecond = (WORD) ptm->tm_sec;
	timeDest.wMilliseconds = 0;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// String formatting

#define maxTimeBufferSize       128
	// Verifies will fail if the needed buffer size is too large

inline CxString CxTimeSpan::Format(LPCTSTR pFormat) const
// formatting timespans is a little trickier than formatting CTimes
//  * we are only interested in relative time formats, ie. it is illegal
//      to format anything dealing with absolute time (i.e. years, months,
//         day of week, day of year, timezones, ...)
//  * the only valid formats:
//      %D - # of days -- NEW !!!
//      %H - hour in 24 hour format
//      %M - minute (0-59)
//      %S - seconds (0-59)
//      %% - percent sign
{
	TCHAR szBuffer[maxTimeBufferSize];
	TCHAR ch;
	LPTSTR pch = szBuffer;

	while ((ch = *pFormat++) != '\0')
	{
		XASSERT(pch < &szBuffer[maxTimeBufferSize]);
		if (ch == '%')
		{
			switch (ch = *pFormat++)
			{
			default:
				XASSERT(FALSE);      // probably a bad format character
			case '%':
				*pch++ = ch;
				break;
			case 'D':
				pch += wsprintf(pch, _T("%ld"), GetDays());
				break;
			case 'H':
				pch += wsprintf(pch, _T("%02d"), GetHours());
				break;
			case 'M':
				pch += wsprintf(pch, _T("%02d"), GetMinutes());
				break;
			case 'S':
				pch += wsprintf(pch, _T("%02d"), GetSeconds());
				break;
			}
		}
		else
		{
			*pch++ = ch;
			if (_istlead(ch))
			{
				XASSERT(pch < &szBuffer[maxTimeBufferSize]);
				*pch++ = *pFormat++;
			}
		}
	}

	*pch = '\0';
	return szBuffer;
}

inline CxString CxTime::Format(LPCTSTR pFormat) const
{
	TCHAR szBuffer[maxTimeBufferSize];

	struct tm _tm;
	errno_t err;
	err = localtime_s(&_tm, &m_time);
	if (err ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, &_tm))
		szBuffer[0] = '\0';
	return szBuffer;
}

inline CxString CxTime::FormatGmt(LPCTSTR pFormat) const
{
	TCHAR szBuffer[maxTimeBufferSize];

	errno_t err;
	struct tm _tm;
	err = gmtime_s(&_tm, &m_time);
	if (err ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, &_tm))
		szBuffer[0] = '\0';
	return szBuffer;
}
