// xTime.inl: implementation of the CxTime class.
//
//////////////////////////////////////////////////////////////////////

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
		*ptm = *gmtime(&m_time);
		return ptm;
	}
	else
		return gmtime(&m_time);
}

inline struct tm* CxTime::GetLocalTm(struct tm* ptm) const
{
	if (ptm != NULL)
	{
		struct tm* ptmTemp = localtime(&m_time);
		if (ptmTemp == NULL)
			return NULL;    // indicates the m_time was not initialized!

		*ptm = *ptmTemp;
		return ptm;
	}
	else
		return localtime(&m_time);
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

	struct tm* ptmTemp = localtime(&m_time);
	if (ptmTemp == NULL ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}

inline CxString CxTime::FormatGmt(LPCTSTR pFormat) const
{
	TCHAR szBuffer[maxTimeBufferSize];

	struct tm* ptmTemp = gmtime(&m_time);
	if (ptmTemp == NULL ||
		!_tcsftime(szBuffer, _countof(szBuffer), pFormat, ptmTemp))
		szBuffer[0] = '\0';
	return szBuffer;
}
