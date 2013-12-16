// xOleDateTime.cpp: implementation of the CxOleDateTime class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <time.h>
#include <math.h>
#include <wtypes.h>

#include <XUtil/xOleDateTime.h>
#include <XUtil/xException.h>

// Return the highest order bit composing dwTarget in wBit
#ifndef HI_BIT
#define HI_BIT(dwTarget, wBit) \
	do \
	{ \
	if (dwTarget != 0) \
	for (wBit = 32; (dwTarget & (0x00000001 << (wBit-1))) == 0; wBit--);\
	else \
	wBit = 0; \
	} while (0)
#endif

// Left shift an (assumed unsigned) currency by wBits
#ifndef LSHIFT_UCUR
#define LSHIFT_UCUR(cur, wBits) \
	do \
	{ \
	for (WORD wTempBits = wBits; wTempBits > 0; wTempBits--) \
		{ \
		cur.m_cur.Hi = ((DWORD)cur.m_cur.Hi << 1); \
		cur.m_cur.Hi |= (cur.m_cur.Lo & 0x80000000) >> 31; \
		cur.m_cur.Lo = cur.m_cur.Lo << 1; \
		} \
	} while (0)
#endif

// Right shift an (assumed unsigned) currency by wBits
#ifndef RSHIFT_UCUR
#define RSHIFT_UCUR(cur, wBits) \
	do \
	{ \
	for (WORD wTempBits = wBits; wTempBits > 0; wTempBits--) \
		{ \
		cur.m_cur.Lo = cur.m_cur.Lo >> 1; \
		cur.m_cur.Lo |= (cur.m_cur.Hi & 0x00000001) << 31; \
		cur.m_cur.Hi = ((DWORD)cur.m_cur.Hi >> 1); \
		} \
	} while (0)
#endif

#if defined(_UNICODE) || defined(OLE2ANSI)
#ifndef VTS_BSTR
#define VTS_BSTR            VTS_WBSTR// an 'LPCOLESTR'
#endif
#ifndef VT_BSTRT
#define VT_BSTRT            VT_BSTR
#endif
#else
#ifndef VTS_BSTR
#define VTS_BSTR            "\x0E"  // an 'LPCSTR'
#endif
#ifndef VT_BSTRA
#define VT_BSTRA            14
#endif
#ifndef VT_BSTRT
#define VT_BSTRT            VT_BSTRA
#endif
#endif

#ifndef UNUSED
#define UNUSED(x) x
#endif

// Verifies will fail if the needed buffer size is too large
#define MAX_TIME_BUFFER_SIZE    128         // matches that in timecore.cpp
#define MIN_DATE                (-657434L)  // about year 100
#define MAX_DATE                2958465L    // about year 9999

// Half a second, expressed in days
#define HALF_SECOND  (1.0/172800.0)

// One-based array of days in year at month start
int _MonthDays[13] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

void _CheckError(SCODE sc)
{
	if ( FAILED(sc) )
	{
		throw CxException( _T("CxOleVariant"), _T("ERROR") );
	}
}

void _VariantInit(LPVARIANT pVar)
{
	memset(pVar, 0, sizeof(*pVar));
}

BOOL _CompareSafeArrays(SAFEARRAY* parray1, SAFEARRAY* parray2)
{
	BOOL bCompare = FALSE;
	
	// If one is NULL they must both be NULL to compare
	if (parray1 == NULL || parray2 == NULL)
	{
		return parray1 == parray2;
	}
	
	// Dimension must match and if 0, then arrays compare
	DWORD dwDim1 = ::SafeArrayGetDim(parray1);
	DWORD dwDim2 = ::SafeArrayGetDim(parray2);
	if (dwDim1 != dwDim2)
		return FALSE;
	else if (dwDim1 == 0)
		return TRUE;
	
	// Element size must match
	DWORD dwSize1 = ::SafeArrayGetElemsize(parray1);
	DWORD dwSize2 = ::SafeArrayGetElemsize(parray2);
	if (dwSize1 != dwSize2)
		return FALSE;
	
	long* pLBound1 = NULL;
	long* pLBound2 = NULL;
	long* pUBound1 = NULL;
	long* pUBound2 = NULL;
	
	void* pData1 = NULL;
	void* pData2 = NULL;
	
	try
	{
		// Bounds must match
		pLBound1 = new long[dwDim1];
		pLBound2 = new long[dwDim2];
		pUBound1 = new long[dwDim1];
		pUBound2 = new long[dwDim2];
		
		size_t nTotalElements = 1;
		
		// Get and compare bounds
		for (DWORD dwIndex = 0; dwIndex < dwDim1; dwIndex++)
		{
			_CheckError(::SafeArrayGetLBound(
				parray1, dwIndex+1, &pLBound1[dwIndex]));
			_CheckError(::SafeArrayGetLBound(
				parray2, dwIndex+1, &pLBound2[dwIndex]));
			_CheckError(::SafeArrayGetUBound(
				parray1, dwIndex+1, &pUBound1[dwIndex]));
			_CheckError(::SafeArrayGetUBound(
				parray2, dwIndex+1, &pUBound2[dwIndex]));
			
			// Check the magnitude of each bound
			if (pUBound1[dwIndex] - pLBound1[dwIndex] !=
				pUBound2[dwIndex] - pLBound2[dwIndex])
			{
				delete[] pLBound1;
				delete[] pLBound2;
				delete[] pUBound1;
				delete[] pUBound2;
				
				return FALSE;
			}
			
			// Increment the element count
			nTotalElements *= pUBound1[dwIndex] - pLBound1[dwIndex] + 1;
		}
		
		// Access the data
		_CheckError(::SafeArrayAccessData(parray1, &pData1));
		_CheckError(::SafeArrayAccessData(parray2, &pData2));
		
		// Calculate the number of bytes of data and compare
		size_t nSize = nTotalElements * dwSize1;
		int nOffset = memcmp(pData1, pData2, nSize);
		bCompare = nOffset == 0;
		
		// Release the array locks
		_CheckError(::SafeArrayUnaccessData(parray1));
		_CheckError(::SafeArrayUnaccessData(parray2));
	}
	catch(...)
	{
		// Clean up bounds arrays
		delete[] pLBound1;
		delete[] pLBound2;
		delete[] pUBound1;
		delete[] pUBound2;
		
		// Release the array locks
		if (pData1 != NULL)
			_CheckError(::SafeArrayUnaccessData(parray1));
		if (pData2 != NULL)
			_CheckError(::SafeArrayUnaccessData(parray2));
		
		throw CxException( _T("_CompareSafeArrays"), _T("ERROR") );
	}
	
	// Clean up bounds arrays
	delete[] pLBound1;
	delete[] pLBound2;
	delete[] pUBound1;
	delete[] pUBound2;
	
	return bCompare;
}

DATE _DateFromDouble(double dbl)
{
	// No problem if positive
	if (dbl >= 0)
		return dbl;
	
	// If negative, must convert since negative dates not continuous
	// (examples: -.75 to -1.25, -.50 to -1.50, -.25 to -1.75)
	double temp = floor(dbl); // dbl is now whole part
	return temp + (temp - dbl);
}

double _DoubleFromDate(DATE dt)
{
	// No problem if positive
	if (dt >= 0)
		return dt;
	
	// If negative, must convert since negative dates not continuous
	// (examples: -1.25 to -.75, -1.50 to -.50, -1.75 to -.25)
	double temp = ceil(dt);
	return temp - (dt - temp);
}

void _TmConvertToStandardFormat(struct tm& tmSrc)
{
	// Convert afx internal tm to format expected by runtimes (_tcsftime, etc)
	tmSrc.tm_year -= 1900;  // year is based on 1900
	tmSrc.tm_mon -= 1;      // month of year is 0-based
	tmSrc.tm_wday -= 1;     // day of week is 0-based
	tmSrc.tm_yday -= 1;     // day of year is 0-based
}

BOOL _OleDateFromTm(WORD wYear, WORD wMonth, WORD wDay,
					WORD wHour, WORD wMinute, WORD wSecond, DATE& dtDest)
{
	// Validate year and month (ignore day of week and milliseconds)
	if (wYear > 9999 || wMonth < 1 || wMonth > 12)
		return FALSE;
	
	//  Check for leap year and set the number of days in the month
	BOOL bLeapYear = ((wYear & 3) == 0) &&
		((wYear % 100) != 0 || (wYear % 400) == 0);
	
	int nDaysInMonth =
		_MonthDays[wMonth] - _MonthDays[wMonth-1] +
		((bLeapYear && wDay == 29 && wMonth == 2) ? 1 : 0);
	
	// Finish validating the date
	if (wDay < 1 || wDay > nDaysInMonth ||
		wHour > 23 || wMinute > 59 ||
		wSecond > 59)
	{
		return FALSE;
	}
	
	// Cache the date in days and time in fractional days
	long nDate;
	double dblTime;
	
	//It is a valid date; make Jan 1, 1AD be 1
	nDate = wYear*365L + wYear/4 - wYear/100 + wYear/400 +
		_MonthDays[wMonth-1] + wDay;
	
	//  If leap year and it's before March, subtract 1:
	if (wMonth <= 2 && bLeapYear)
		--nDate;
	
	//  Offset so that 12/30/1899 is 0
	nDate -= 693959L;
	
	dblTime = (((long)wHour * 3600L) +  // hrs in seconds
		((long)wMinute * 60L) +  // mins in seconds
		((long)wSecond)) / 86400.;
	
	dtDest = (double) nDate + ((nDate >= 0) ? dblTime : -dblTime);
	
	return TRUE;
}

BOOL _TmFromOleDate(DATE dtSrc, struct tm& tmDest)
{
	// The legal range does not actually span year 0 to 9999.
	if (dtSrc > MAX_DATE || dtSrc < MIN_DATE) // about year 100 to about 9999
		return FALSE;
	
	long nDays;             // Number of days since Dec. 30, 1899
	long nDaysAbsolute;     // Number of days since 1/1/0
	long nSecsInDay;        // Time in seconds since midnight
	long nMinutesInDay;     // Minutes in day
	
	long n400Years;         // Number of 400 year increments since 1/1/0
	long n400Century;       // Century within 400 year block (0,1,2 or 3)
	long n4Years;           // Number of 4 year increments since 1/1/0
	long n4Day;             // Day within 4 year block
	//  (0 is 1/1/yr1, 1460 is 12/31/yr4)
	long n4Yr;              // Year within 4 year block (0,1,2 or 3)
	BOOL bLeap4 = TRUE;     // TRUE if 4 year block includes leap year
	
	double dblDate = dtSrc; // tempory serial date
	
	// If a valid date, then this conversion should not overflow
	nDays = (long)dblDate;
	
	// Round to the second
	dblDate += ((dtSrc > 0.0) ? HALF_SECOND : -HALF_SECOND);
	
	nDaysAbsolute = (long)dblDate + 693959L; // Add days from 1/1/0 to 12/30/1899
	
	dblDate = fabs(dblDate);
	nSecsInDay = (long)((dblDate - floor(dblDate)) * 86400.);
	
	// Calculate the day of week (sun=1, mon=2...)
	//   -1 because 1/1/0 is Sat.  +1 because we want 1-based
	tmDest.tm_wday = (int)((nDaysAbsolute - 1) % 7L) + 1;
	
	// Leap years every 4 yrs except centuries not multiples of 400.
	n400Years = (long)(nDaysAbsolute / 146097L);
	
	// Set nDaysAbsolute to day within 400-year block
	nDaysAbsolute %= 146097L;
	
	// -1 because first century has extra day
	n400Century = (long)((nDaysAbsolute - 1) / 36524L);
	
	// Non-leap century
	if (n400Century != 0)
	{
		// Set nDaysAbsolute to day within century
		nDaysAbsolute = (nDaysAbsolute - 1) % 36524L;
		
		// +1 because 1st 4 year increment has 1460 days
		n4Years = (long)((nDaysAbsolute + 1) / 1461L);
		
		if (n4Years != 0)
			n4Day = (long)((nDaysAbsolute + 1) % 1461L);
		else
		{
			bLeap4 = FALSE;
			n4Day = (long)nDaysAbsolute;
		}
	}
	else
	{
		// Leap century - not special case!
		n4Years = (long)(nDaysAbsolute / 1461L);
		n4Day = (long)(nDaysAbsolute % 1461L);
	}
	
	if (bLeap4)
	{
		// -1 because first year has 366 days
		n4Yr = (n4Day - 1) / 365;
		
		if (n4Yr != 0)
			n4Day = (n4Day - 1) % 365;
	}
	else
	{
		n4Yr = n4Day / 365;
		n4Day %= 365;
	}
	
	// n4Day is now 0-based day of year. Save 1-based day of year, year number
	tmDest.tm_yday = (int)n4Day + 1;
	tmDest.tm_year = n400Years * 400 + n400Century * 100 + n4Years * 4 + n4Yr;
	
	// Handle leap year: before, on, and after Feb. 29.
	if (n4Yr == 0 && bLeap4)
	{
		// Leap Year
		if (n4Day == 59)
		{
			/* Feb. 29 */
			tmDest.tm_mon = 2;
			tmDest.tm_mday = 29;
			goto DoTime;
		}
		
		// Pretend it's not a leap year for month/day comp.
		if (n4Day >= 60)
			--n4Day;
	}
	
	// Make n4DaY a 1-based day of non-leap year and compute
	//  month/day for everything but Feb. 29.
	++n4Day;
	
	// Month number always >= n/32, so save some loop time */
	for (tmDest.tm_mon = (n4Day >> 5) + 1;
	n4Day > _MonthDays[tmDest.tm_mon]; tmDest.tm_mon++);
	
	tmDest.tm_mday = (int)(n4Day - _MonthDays[tmDest.tm_mon-1]);
	
DoTime:
	if (nSecsInDay == 0)
		tmDest.tm_hour = tmDest.tm_min = tmDest.tm_sec = 0;
	else
	{
		tmDest.tm_sec = (int)nSecsInDay % 60L;
		nMinutesInDay = nSecsInDay / 60L;
		tmDest.tm_min = (int)nMinutesInDay % 60;
		tmDest.tm_hour = (int)nMinutesInDay / 60;
	}
	
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CxOleDateTime class

CxOleDateTime PASCAL CxOleDateTime::GetCurrentTime()
{
	return CxOleDateTime(::time(NULL));
}

BOOL CxOleDateTime::GetAsSystemTime(SYSTEMTIME& sysTime) const
{
	BOOL bRetVal = FALSE;
	if (GetStatus() == valid)
	{
		struct tm tmTemp;
		if (_TmFromOleDate(m_dt, tmTemp))
		{
			sysTime.wYear = (WORD) tmTemp.tm_year;
			sysTime.wMonth = (WORD) tmTemp.tm_mon;
			sysTime.wDayOfWeek = (WORD) (tmTemp.tm_wday - 1);
			sysTime.wDay = (WORD) tmTemp.tm_mday;
			sysTime.wHour = (WORD) tmTemp.tm_hour;
			sysTime.wMinute = (WORD) tmTemp.tm_min;
			sysTime.wSecond = (WORD) tmTemp.tm_sec;
			sysTime.wMilliseconds = 0;
			
			bRetVal = TRUE;
		}
	}
	
	return bRetVal;
}

int CxOleDateTime::GetYear() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_year;
	else
		return -1;
}

int CxOleDateTime::GetMonth() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_mon;
	else
		return -1;
}

int CxOleDateTime::GetDay() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_mday;
	else
		return -1;
}

int CxOleDateTime::GetHour() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_hour;
	else
		return -1;
}

int CxOleDateTime::GetMinute() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_min;
	else
		return -1;
}

int CxOleDateTime::GetSecond() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_sec;
	else
		return -1;
}

int CxOleDateTime::GetDayOfWeek() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_wday;
	else
		return -1;
}

int CxOleDateTime::GetDayOfYear() const
{
	struct tm tmTemp;
	
	if (GetStatus() == valid && _TmFromOleDate(m_dt, tmTemp))
		return tmTemp.tm_yday;
	else
		return -1;
}

const CxOleDateTime& CxOleDateTime::operator = (const VARIANT& varSrc)
{
	if (varSrc.vt != VT_DATE)
	{
		try
		{
			CxOleVariant varTemp(varSrc);
			varTemp.ChangeType(VT_DATE);
			m_dt = varTemp.date;
			SetStatus(valid);
		}
		// Catch CxException from ChangeType, but not CMemoryException
		catch (CxException e)
		{
			m_dt = 0;
			SetStatus(invalid);
		}
	}
	else
	{
		m_dt = varSrc.date;
		SetStatus(valid);
	}
	
	return *this;
}

const CxOleDateTime& CxOleDateTime::operator=(DATE dtSrc)
{
	m_dt = dtSrc;
	SetStatus(valid);
	
	return *this;
}

const CxOleDateTime& CxOleDateTime::operator=(const time_t& timeSrc)
{
	// Convert time_t to struct tm
	tm *ptm = localtime(&timeSrc);
	
	if (ptm != NULL)
	{
		m_status = _OleDateFromTm((WORD)(ptm->tm_year + 1900),
			(WORD)(ptm->tm_mon + 1), (WORD)ptm->tm_mday,
			(WORD)ptm->tm_hour, (WORD)ptm->tm_min,
			(WORD)ptm->tm_sec, m_dt) ? valid : invalid;
	}
	else
	{
		// Local time must have failed (timsSrc before 1/1/70 12am)
		SetStatus(invalid);
		XASSERT(FALSE);
	}
	
	return *this;
}

const CxOleDateTime& CxOleDateTime::operator=(const SYSTEMTIME& systimeSrc)
{
	m_status = _OleDateFromTm(systimeSrc.wYear, systimeSrc.wMonth,
		systimeSrc.wDay, systimeSrc.wHour, systimeSrc.wMinute,
		systimeSrc.wSecond, m_dt) ? valid : invalid;
	
	return *this;
}

const CxOleDateTime& CxOleDateTime::operator=(const FILETIME& filetimeSrc)
{
	// Assume UTC FILETIME, so convert to LOCALTIME
	FILETIME filetimeLocal;
	if (!FileTimeToLocalFileTime( &filetimeSrc, &filetimeLocal))
	{
#ifdef _DEBUG
		DWORD dwError = GetLastError();
		XTRACE(_T("\nFileTimeToLocalFileTime failed. Error = %lu.\n\t"), dwError);
#endif // _DEBUG
		m_status = invalid;
	}
	else
	{
		// Take advantage of SYSTEMTIME -> FILETIME conversion
		SYSTEMTIME systime;
		m_status = FileTimeToSystemTime(&filetimeLocal, &systime) ?
valid : invalid;
		
		// At this point systime should always be valid, but...
		if (GetStatus() == valid)
		{
			m_status = _OleDateFromTm(systime.wYear, systime.wMonth,
				systime.wDay, systime.wHour, systime.wMinute,
				systime.wSecond, m_dt) ? valid : invalid;
		}
	}
	
	return *this;
}

BOOL CxOleDateTime::operator<(const CxOleDateTime& date) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(date.GetStatus() == valid);
	
	// Handle negative dates
	return _DoubleFromDate(m_dt) < _DoubleFromDate(date.m_dt);
}

BOOL CxOleDateTime::operator>(const CxOleDateTime& date) const
{   XASSERT(GetStatus() == valid);
XASSERT(date.GetStatus() == valid);

// Handle negative dates
return _DoubleFromDate(m_dt) > _DoubleFromDate(date.m_dt);
}

BOOL CxOleDateTime::operator<=(const CxOleDateTime& date) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(date.GetStatus() == valid);
	
	// Handle negative dates
	return _DoubleFromDate(m_dt) <= _DoubleFromDate(date.m_dt);
}

BOOL CxOleDateTime::operator>=(const CxOleDateTime& date) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(date.GetStatus() == valid);
	
	// Handle negative dates
	return _DoubleFromDate(m_dt) >= _DoubleFromDate(date.m_dt);
}

CxOleDateTime CxOleDateTime::operator+(const CxOleDateTimeSpan& dateSpan) const
{
	CxOleDateTime dateResult;    // Initializes m_status to valid
	
	// If either operand NULL, result NULL
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateResult.SetStatus(null);
		return dateResult;
	}
	
	// If either operand invalid, result invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateResult.SetStatus(invalid);
		return dateResult;
	}
	
	// Compute the actual date difference by adding underlying dates
	dateResult = _DateFromDouble(_DoubleFromDate(m_dt) + dateSpan.m_span);
	
	// Validate within range
	dateResult.CheckRange();
	
	return dateResult;
}

CxOleDateTime CxOleDateTime::operator-(const CxOleDateTimeSpan& dateSpan) const
{
	CxOleDateTime dateResult;    // Initializes m_status to valid
	
	// If either operand NULL, result NULL
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateResult.SetStatus(null);
		return dateResult;
	}
	
	// If either operand invalid, result invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateResult.SetStatus(invalid);
		return dateResult;
	}
	
	// Compute the actual date difference by subtracting underlying dates
	dateResult = _DateFromDouble(_DoubleFromDate(m_dt) - dateSpan.m_span);
	
	// Validate within range
	dateResult.CheckRange();
	
	return dateResult;
}

CxOleDateTimeSpan CxOleDateTime::operator-(const CxOleDateTime& date) const
{
	CxOleDateTimeSpan spanResult;
	
	// If either operand NULL, result NULL
	if (GetStatus() == null || date.GetStatus() == null)
	{
		spanResult.SetStatus(CxOleDateTimeSpan::null);
		return spanResult;
	}
	
	// If either operand invalid, result invalid
	if (GetStatus() == invalid || date.GetStatus() == invalid)
	{
		spanResult.SetStatus(CxOleDateTimeSpan::invalid);
		return spanResult;
	}
	
	// Return result (span can't be invalid, so don't check range)
	return _DoubleFromDate(m_dt) - _DoubleFromDate(date.m_dt);
}

int CxOleDateTime::SetDateTime(int nYear, int nMonth, int nDay,
							   int nHour, int nMin, int nSec)
{
	return m_status = _OleDateFromTm((WORD)nYear, (WORD)nMonth,
		(WORD)nDay, (WORD)nHour, (WORD)nMin, (WORD)nSec, m_dt) ?
valid : invalid;
}

BOOL CxOleDateTime::ParseDateTime(LPCTSTR lpszDate, DWORD dwFlags, LCID lcid)
{
	USES_CONVERSION;
	CxString strDate = lpszDate;
	
	SCODE sc;
	if (FAILED(sc = VarDateFromStr((LPOLESTR)T2COLE(strDate), lcid,
		dwFlags, &m_dt)))
	{
		if (sc == DISP_E_TYPEMISMATCH)
		{
			// Can't convert string to date, set 0 and invalidate
			m_dt = 0;
			SetStatus(invalid);
			return FALSE;
		}
		else if (sc == DISP_E_OVERFLOW)
		{
			// Can't convert string to date, set -1 and invalidate
			m_dt = -1;
			SetStatus(invalid);
			return FALSE;
		}
		else
		{
			XTRACE(_T("\nCxOleDateTime VarDateFromStr call failed.\n\t"));
			if (sc == E_OUTOFMEMORY)
			{
				throw CxException( _T("CxOleDateTime"), _T("Out of memory") );
			}
			else
			{
				throw CxException( _T("CxOleDateTime"), _T("Error") );
			}
		}
	}
	
	SetStatus(valid);
	return TRUE;
}

CxString CxOleDateTime::Format(DWORD dwFlags, LCID lcid) const
{
	USES_CONVERSION;
	CxString strDate;
	
	// If null, return empty string
	if (GetStatus() == null)
		return strDate;
	
	// If invalid, return DateTime resource string
	if (GetStatus() == invalid)
	{
		strDate = _T("Invalid Datetime");
		//XVERIFY(strDate.LoadString(AFX_IDS_INVALID_DATETIME));
		return strDate;
	}
	
	CxOleVariant var;
	// Don't need to trap error. Should not fail due to type mismatch
	_CheckError(VarBstrFromDate(m_dt, lcid, dwFlags, &V_BSTR(&var)));
	var.vt = VT_BSTR;
	return OLE2CT(V_BSTR(&var));
}

CxString CxOleDateTime::Format(LPCTSTR pFormat) const
{
	CxString strDate;
	struct tm tmTemp;
	
	// If null, return empty string
	if (GetStatus() == null)
		return strDate;
	
	// If invalid, return DateTime resource string
	if (GetStatus() == invalid || !_TmFromOleDate(m_dt, tmTemp))
	{
		strDate = _T("Invalid datetime");
		//XVERIFY(strDate.LoadString(AFX_IDS_INVALID_DATETIME));
		return strDate;
	}
	
	// Convert tm from afx internal format to standard format
	_TmConvertToStandardFormat(tmTemp);
	
	// Fill in the buffer, disregard return value as it's not necessary
	LPTSTR lpszTemp = strDate.GetBufferSetLength(MAX_TIME_BUFFER_SIZE);
	_tcsftime(lpszTemp, strDate.GetLength(), pFormat, &tmTemp);
	strDate.ReleaseBuffer();
	
	return strDate;
}

void CxOleDateTime::CheckRange()
{
	if (m_dt > MAX_DATE || m_dt < MIN_DATE) // about year 100 to about 9999
		SetStatus(invalid);
}

/////////////////////////////////////////////////////////////////////////////
// CxOleDateTimeSpan class helpers

#define MAX_DAYS_IN_SPAN    3615897L

/////////////////////////////////////////////////////////////////////////////
// CxOleDateTimeSpan class
long CxOleDateTimeSpan::GetHours() const
{
	XASSERT(GetStatus() == valid);
	
	double dblTemp;
	
	// Truncate days and scale up
	dblTemp = modf(m_span, &dblTemp);
	
	long lReturns = (long)((dblTemp + _OLE_DATETIME_HALFSECOND) * 24);
	if (lReturns >= 24)
		lReturns -= 24;
	
	return lReturns;
}

long CxOleDateTimeSpan::GetMinutes() const
{
	XASSERT(GetStatus() == valid);
	
	double dblTemp;
	
	// Truncate hours and scale up
	dblTemp = modf(m_span * 24, &dblTemp);
	
	long lReturns = (long) ((dblTemp + _OLE_DATETIME_HALFSECOND) * 60);
	if (lReturns >= 60)
		lReturns -= 60;
	
	return lReturns;
}

long CxOleDateTimeSpan::GetSeconds() const
{
	XASSERT(GetStatus() == valid);
	
	double dblTemp;
	
	// Truncate minutes and scale up
	dblTemp = modf(m_span * 24 * 60, &dblTemp);
	
	long lReturns = (long) ((dblTemp + _OLE_DATETIME_HALFSECOND) * 60);
	if (lReturns >= 60)
		lReturns -= 60;
	
	return lReturns;
}

const CxOleDateTimeSpan& CxOleDateTimeSpan::operator=(double dblSpanSrc)
{
	m_span = dblSpanSrc;
	SetStatus(valid);
	return *this;
}

const CxOleDateTimeSpan& CxOleDateTimeSpan::operator=(const CxOleDateTimeSpan& dateSpanSrc)
{
	m_span = dateSpanSrc.m_span;
	m_status = dateSpanSrc.m_status;
	return *this;
}

CxOleDateTimeSpan CxOleDateTimeSpan::operator+(const CxOleDateTimeSpan& dateSpan) const
{
	CxOleDateTimeSpan dateSpanTemp;
	
	// If either operand Null, result Null
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateSpanTemp.SetStatus(null);
		return dateSpanTemp;
	}
	
	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateSpanTemp.SetStatus(invalid);
		return dateSpanTemp;
	}
	
	// Add spans and validate within legal range
	dateSpanTemp.m_span = m_span + dateSpan.m_span;
	dateSpanTemp.CheckRange();
	
	return dateSpanTemp;
}

CxOleDateTimeSpan CxOleDateTimeSpan::operator-(const CxOleDateTimeSpan& dateSpan) const
{
	CxOleDateTimeSpan dateSpanTemp;
	
	// If either operand Null, result Null
	if (GetStatus() == null || dateSpan.GetStatus() == null)
	{
		dateSpanTemp.SetStatus(null);
		return dateSpanTemp;
	}
	
	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || dateSpan.GetStatus() == invalid)
	{
		dateSpanTemp.SetStatus(invalid);
		return dateSpanTemp;
	}
	
	// Subtract spans and validate within legal range
	dateSpanTemp.m_span = m_span - dateSpan.m_span;
	dateSpanTemp.CheckRange();
	
	return dateSpanTemp;
}

void CxOleDateTimeSpan::SetDateTimeSpan(
										long lDays, int nHours, int nMins, int nSecs)
{
	// Set date span by breaking into fractional days (all input ranges valid)
	m_span = lDays + ((double)nHours)/24 + ((double)nMins)/(24*60) +
		((double)nSecs)/(24*60*60);
	
	SetStatus(valid);
}

CxString CxOleDateTimeSpan::Format(LPCTSTR pFormat) const
{
	CxString strSpan;
	struct tm tmTemp;
	
	// If null, return empty string
	if (GetStatus() == null)
		return strSpan;
	
	// If invalid, return DateTimeSpan resource string
	if (GetStatus() == invalid || !_TmFromOleDate(m_span, tmTemp))
	{
		strSpan = _T("Invalid Datetimespan");
		//XVERIFY(strSpan.LoadString(AFX_IDS_INVALID_DATETIMESPAN));
		return strSpan;
	}
	
	// Convert tm from afx internal format to standard format
	_TmConvertToStandardFormat(tmTemp);
	
	// _tcsftime() doesn't handle %D, so do it here
	
	CxString strPreParsed;
	LPCTSTR pstrSource = pFormat;
	int nTargetChar = 0;
	int nAccumulatedLength = lstrlen(pFormat);
	LPTSTR pstrTarget = strPreParsed.GetBuffer(nAccumulatedLength);
	
	while (*pstrSource)
	{
		if (*pstrSource == '%' && pstrSource[1] == 'D')
		{
			TCHAR szDay[12];
			_itot(GetDays(), szDay, 10);
			strPreParsed.ReleaseBuffer(nTargetChar);
			strPreParsed += szDay;
			int nTemp = lstrlen(szDay);
			nAccumulatedLength += nTemp;
			nTargetChar += nTemp;
			pstrTarget = strPreParsed.GetBuffer(nAccumulatedLength)
				+ nTargetChar;
			pstrSource = _tcsinc(pstrSource);
			pstrSource = _tcsinc(pstrSource);
		}
		*pstrTarget = *pstrSource;
		nTargetChar++;
		pstrSource = _tcsinc(pstrSource);
		pstrTarget = _tcsinc(pstrTarget);
	}
	strPreParsed.ReleaseBuffer(nTargetChar);
	
	// Fill in the buffer, disregard return value as it's not necessary
	LPTSTR lpszTemp = strSpan.GetBufferSetLength(MAX_TIME_BUFFER_SIZE);
	_tcsftime(lpszTemp, strSpan.GetLength(), (LPCTSTR) strPreParsed, &tmTemp);
	strSpan.ReleaseBuffer();
	
	return strSpan;
}

void CxOleDateTimeSpan::CheckRange()
{
	if (m_span < -MAX_DAYS_IN_SPAN || m_span > MAX_DAYS_IN_SPAN)
		SetStatus(invalid);
}

/////////////////////////////////////////////////////////////////////////////
// CxOleVariant class

typedef VARIANT __RPC_FAR* LPVARIANT;

CxOleVariant::CxOleVariant(const VARIANT& varSrc)
{
	_VariantInit(this);
	_CheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
}

CxOleVariant::CxOleVariant(LPCVARIANT pSrc)
{
	_VariantInit(this);
	_CheckError(::VariantCopy(this, (LPVARIANT)pSrc));
}

CxOleVariant::CxOleVariant(const CxOleVariant& varSrc)
{
	_VariantInit(this);
	_CheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
}

CxOleVariant::CxOleVariant(LPCTSTR lpszSrc, VARTYPE vtSrc)
{
	USES_CONVERSION;
	XASSERT(vtSrc == VT_BSTR || vtSrc == VT_BSTRT);
	UNUSED(vtSrc);
	
	vt = VT_BSTR;
	bstrVal = NULL;
	
	if (lpszSrc != NULL)
	{
#ifndef _UNICODE
		if (vtSrc == VT_BSTRT)
		{
			int nLen = lstrlen(lpszSrc);
			bstrVal = ::SysAllocStringByteLen(lpszSrc, nLen);
		}
		else
#endif
		{
			bstrVal = ::SysAllocString(T2COLE(lpszSrc));
		}
		
		if (bstrVal == NULL)
		{
			throw CxException( _T("CxOleVariant"), _T("Memory Exception") );
			//AfxThrowMemoryException();
		}
	}
}



void CxOleVariant::SetString(LPCTSTR lpszSrc, VARTYPE vtSrc)
{
	USES_CONVERSION;
	XASSERT(vtSrc == VT_BSTR || vtSrc == VT_BSTRT);
	UNUSED(vtSrc);
	
	// Free up previous VARIANT
	Clear();
	
	vt = VT_BSTR;
	bstrVal = NULL;
	
	if (lpszSrc != NULL)
	{
#ifndef _UNICODE
		if (vtSrc == VT_BSTRT)
		{
			int nLen = lstrlen(lpszSrc);
			bstrVal = ::SysAllocStringByteLen(lpszSrc, nLen);
		}
		else
#endif
		{
			bstrVal = ::SysAllocString(T2COLE(lpszSrc));
		}
		
		if (bstrVal == NULL)
			throw CxException( _T("CxOleVariant"), _T("Memory Exception") );
	}
}

CxOleVariant::CxOleVariant(short nSrc, VARTYPE vtSrc)
{
	XASSERT(vtSrc == VT_I2 || vtSrc == VT_BOOL);
	
	if (vtSrc == VT_BOOL)
	{
		vt = VT_BOOL;
		if (!nSrc)
			V_BOOL(this) = 0;
		else
			V_BOOL(this) = -1;
	}
	else
	{
		vt = VT_I2;
		iVal = nSrc;
	}
}

CxOleVariant::CxOleVariant(long lSrc, VARTYPE vtSrc)
{
	XASSERT(vtSrc == VT_I4 || vtSrc == VT_ERROR || vtSrc == VT_BOOL);
	
	if (vtSrc == VT_ERROR)
	{
		vt = VT_ERROR;
		scode = lSrc;
	}
	else if (vtSrc == VT_BOOL)
	{
		vt = VT_BOOL;
		if (!lSrc)
			V_BOOL(this) = 0;
		else
			V_BOOL(this) = -1;
	}
	else
	{
		vt = VT_I4;
		lVal = lSrc;
	}
}

// Operations
void CxOleVariant::_ClearCompat()
{
	Clear();
}

void CxOleVariant::ChangeType(VARTYPE vartype, LPVARIANT pSrc)
{
	// If pSrc is NULL, convert type in place
	if (pSrc == NULL)
		pSrc = this;
	if (pSrc != this || vartype != vt)
		_CheckError(::VariantChangeType(this, pSrc, 0, vartype));
}

void CxOleVariant::Attach(VARIANT& varSrc)
{
	// Free up previous VARIANT
	Clear();
	
	// give control of data to CxOleVariant
	memcpy(this, &varSrc, sizeof(varSrc));
	varSrc.vt = VT_EMPTY;
}

VARIANT CxOleVariant::Detach()
{
	VARIANT varResult = *this;
	vt = VT_EMPTY;
	return varResult;
}

// Literal comparison. Types and values must match.
BOOL CxOleVariant::operator==(const VARIANT& var) const
{
	if (&var == this)
		return TRUE;
	
	// Variants not equal if types don't match
	if (var.vt != vt)
		return FALSE;
	
	// Check type specific values
	switch (vt)
	{
	case VT_EMPTY:
	case VT_NULL:
		return TRUE;
		
	case VT_BOOL:
		return V_BOOL(&var) == V_BOOL(this);
		
	case VT_UI1:
		return var.bVal == bVal;
		
	case VT_I2:
		return var.iVal == iVal;
		
	case VT_I4:
		return var.lVal == lVal;
		
	case VT_CY:
		return (var.cyVal.Hi == cyVal.Hi && var.cyVal.Lo == cyVal.Lo);
		
	case VT_R4:
		return var.fltVal == fltVal;
		
	case VT_R8:
		return var.dblVal == dblVal;
		
	case VT_DATE:
		return var.date == date;
		
	case VT_BSTR:
		return SysStringByteLen(var.bstrVal) == SysStringByteLen(bstrVal) &&
			memcmp(var.bstrVal, bstrVal, SysStringByteLen(bstrVal)) == 0;
		
	case VT_ERROR:
		return var.scode == scode;
		
	case VT_DISPATCH:
	case VT_UNKNOWN:
		return var.punkVal == punkVal;
		
	default:
		if (vt & VT_ARRAY && !(vt & VT_BYREF))
			return _CompareSafeArrays(var.parray, parray);
		else
			XASSERT(FALSE);  // VT_BYREF not supported
		// fall through
	}
	
	return FALSE;
}

const CxOleVariant& CxOleVariant::operator=(const VARIANT& varSrc)
{
	_CheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(LPCVARIANT pSrc)
{
	_CheckError(::VariantCopy(this, (LPVARIANT)pSrc));
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(const CxOleVariant& varSrc)
{
	_CheckError(::VariantCopy(this, (LPVARIANT)&varSrc));
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(const LPCTSTR lpszSrc)
{
	USES_CONVERSION;
	// Free up previous VARIANT
	Clear();
	
	vt = VT_BSTR;
	if (lpszSrc == NULL)
		bstrVal = NULL;
	else
	{
		bstrVal = ::SysAllocString(T2COLE(lpszSrc));
		if (bstrVal == NULL)
			throw CxException( _T("CxOleVariant"), _T("Memory Exception") );
	}
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(const CxString& strSrc)
{
	USES_CONVERSION;
	// Free up previous VARIANT
	Clear();
	
	vt = VT_BSTR;
	bstrVal = ::SysAllocString(T2COLE(strSrc));
	if (bstrVal == NULL)
		throw CxException( _T("CxOleVariant"), _T("Memory Exception") );
	
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(BYTE nSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_UI1)
	{
		Clear();
		vt = VT_UI1;
	}
	
	bVal = nSrc;
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(short nSrc)
{
	if (vt == VT_I2)
		iVal = nSrc;
	else if (vt == VT_BOOL)
	{
		if (!nSrc)
			V_BOOL(this) = 0;
		else
			V_BOOL(this) = -1;
	}
	else
	{
		// Free up previous VARIANT
		Clear();
		vt = VT_I2;
		iVal = nSrc;
	}
	
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(long lSrc)
{
	if (vt == VT_I4)
		lVal = lSrc;
	else if (vt == VT_ERROR)
		scode = lSrc;
	else if (vt == VT_BOOL)
	{
		if (!lSrc)
			V_BOOL(this) = 0;
		else
			V_BOOL(this) = -1;
	}
	else
	{
		// Free up previous VARIANT
		Clear();
		vt = VT_I4;
		lVal = lSrc;
	}
	
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(const CxOleCurrency& curSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_CY)
	{
		Clear();
		vt = VT_CY;
	}
	
	cyVal = curSrc.m_cur;
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(float fltSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_R4)
	{
		Clear();
		vt = VT_R4;
	}
	
	fltVal = fltSrc;
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(double dblSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_R8)
	{
		Clear();
		vt = VT_R8;
	}
	
	dblVal = dblSrc;
	return *this;
}

const CxOleVariant& CxOleVariant::operator=(const CxOleDateTime& dateSrc)
{
	// Free up previous VARIANT if necessary
	if (vt != VT_DATE)
	{
		Clear();
		vt = VT_DATE;
	}
	
	date = dateSrc.m_dt;
	return *this;
}


/////////////////////////////////////////////////////////////////////////////
// CxOleCurrency class (internally currency is 8-byte int scaled by 10,000)

CxOleCurrency::CxOleCurrency(long nUnits, long nFractionalUnits)
{
	SetCurrency(nUnits, nFractionalUnits);
	SetStatus(valid);
}

const CxOleCurrency& CxOleCurrency::operator=(CURRENCY cySrc)
{
	m_cur = cySrc;
	SetStatus(valid);
	return *this;
}

const CxOleCurrency& CxOleCurrency::operator=(const CxOleCurrency& curSrc)
{
	m_cur = curSrc.m_cur;
	m_status = curSrc.m_status;
	return *this;
}

const CxOleCurrency& CxOleCurrency::operator=(const VARIANT& varSrc)
{
	if (varSrc.vt != VT_CY)
	{
		try
		{
			CxOleVariant varTemp(varSrc);
			varTemp.ChangeType(VT_CY);
			m_cur = varTemp.cyVal;
			SetStatus(valid);
		}
		// Catch COleException from ChangeType, but not CMemoryException
		catch(...)
		{
			// Not able to convert VARIANT to CURRENCY
			m_cur.Hi = 0;
			m_cur.Lo = 0;
			SetStatus(invalid);
		}
	}
	else
	{
		m_cur = varSrc.cyVal;
		SetStatus(valid);
	}
	
	return *this;
}

BOOL CxOleCurrency::operator<(const CxOleCurrency& cur) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(cur.GetStatus() == valid);
	
	return((m_cur.Hi == cur.m_cur.Hi) ?
		(m_cur.Lo < cur.m_cur.Lo) : (m_cur.Hi < cur.m_cur.Hi));
}

BOOL CxOleCurrency::operator>(const CxOleCurrency& cur) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(cur.GetStatus() == valid);
	
	return((m_cur.Hi == cur.m_cur.Hi) ?
		(m_cur.Lo > cur.m_cur.Lo) : (m_cur.Hi > cur.m_cur.Hi));
}

BOOL CxOleCurrency::operator<=(const CxOleCurrency& cur) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(cur.GetStatus() == valid);
	
	return((m_cur.Hi == cur.m_cur.Hi) ?
		(m_cur.Lo <= cur.m_cur.Lo) : (m_cur.Hi < cur.m_cur.Hi));
}

BOOL CxOleCurrency::operator>=(const CxOleCurrency& cur) const
{
	XASSERT(GetStatus() == valid);
	XASSERT(cur.GetStatus() == valid);
	
	return((m_cur.Hi == cur.m_cur.Hi) ?
		(m_cur.Lo >= cur.m_cur.Lo) : (m_cur.Hi > cur.m_cur.Hi));
}

CxOleCurrency CxOleCurrency::operator+(const CxOleCurrency& cur) const
{
	CxOleCurrency curResult;
	
	// If either operand Null, result Null
	if (GetStatus() == null || cur.GetStatus() == null)
	{
		curResult.SetStatus(null);
		return curResult;
	}
	
	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || cur.GetStatus() == invalid)
	{
		curResult.SetStatus(invalid);
		return curResult;
	}
	
	// Add separate CURRENCY components
	curResult.m_cur.Hi = m_cur.Hi + cur.m_cur.Hi;
	curResult.m_cur.Lo = m_cur.Lo + cur.m_cur.Lo;
	
	// Increment Hi if Lo overflows
	if (m_cur.Lo > curResult.m_cur.Lo)
		curResult.m_cur.Hi++;
	
	// Overflow if operands same sign and result sign different
	if (!((m_cur.Hi ^ cur.m_cur.Hi) & 0x80000000) &&
		((m_cur.Hi ^ curResult.m_cur.Hi) & 0x80000000))
	{
		curResult.SetStatus(invalid);
	}
	
	return curResult;
}

CxOleCurrency CxOleCurrency::operator-(const CxOleCurrency& cur) const
{
	CxOleCurrency curResult;
	
	// If either operand Null, result Null
	if (GetStatus() == null || cur.GetStatus() == null)
	{
		curResult.SetStatus(null);
		return curResult;
	}
	
	// If either operand Invalid, result Invalid
	if (GetStatus() == invalid || cur.GetStatus() == invalid)
	{
		curResult.SetStatus(invalid);
		return curResult;
	}
	
	// Subtract separate CURRENCY components
	curResult.m_cur.Hi = m_cur.Hi - cur.m_cur.Hi;
	curResult.m_cur.Lo = m_cur.Lo - cur.m_cur.Lo;
	
	// Decrement Hi if Lo overflows
	if (m_cur.Lo < curResult.m_cur.Lo)
		curResult.m_cur.Hi--;
	
	// Overflow if operands not same sign and result not same sign
	if (((m_cur.Hi ^ cur.m_cur.Hi) & 0x80000000) &&
		((m_cur.Hi ^ curResult.m_cur.Hi) & 0x80000000))
	{
		curResult.SetStatus(invalid);
	}
	
	return curResult;
}

CxOleCurrency CxOleCurrency::operator-() const
{
	// If operand not Valid, just return
	if (!GetStatus() == valid)
		return *this;
	
	CxOleCurrency curResult;
	
	// Negating MIN_CURRENCY,will set invalid
	if (m_cur.Hi == 0x80000000 && m_cur.Lo == 0x00000000)
	{
		curResult.SetStatus(invalid);
	}
	
	curResult.m_cur.Hi = ~m_cur.Hi;
	curResult.m_cur.Lo = -(long)m_cur.Lo;
	
	// If cy was -1 make sure Hi correctly set
	if (curResult.m_cur.Lo == 0)
		curResult.m_cur.Hi++;
	
	return curResult;
}

CxOleCurrency CxOleCurrency::operator*(long nOperand) const
{
	// If operand not Valid, just return
	if (!GetStatus() == valid)
		return *this;
	
	CxOleCurrency curResult(m_cur);
	DWORD nTempOp;
	
	// Return now if one operand is 0 (optimization)
	if ((m_cur.Hi == 0x00000000 && m_cur.Lo == 0x00000000) || nOperand == 0)
	{
		curResult.m_cur.Hi = 0;
		curResult.m_cur.Lo = 0;
		return curResult;
	}
	
	// Handle only valid case of multiplying MIN_CURRENCY
	if (m_cur.Hi == 0x80000000 && m_cur.Lo == 0x00000000 && nOperand == 1)
		return curResult;
	
	// Compute absolute values.
	if (m_cur.Hi < 0)
		curResult = -curResult;
	
	nTempOp = labs(nOperand);
	
	// Check for overflow
	if (curResult.m_cur.Hi != 0)
	{
		WORD wHiBitCur, wHiBitOp;
		HI_BIT(curResult.m_cur.Hi, wHiBitCur);
		HI_BIT(nTempOp, wHiBitOp);
		
		// 63-bit limit on result. (n bits)*(m bits) = (n+m-1) bits.
		if (wHiBitCur + wHiBitOp - 1 > 63)
		{
			// Overflow!
			curResult.SetStatus(invalid);
			
			// Set to maximum negative value
			curResult.m_cur.Hi = 0x80000000;
			curResult.m_cur.Lo = 0x00000000;
			
			return curResult;
		}
	}
	
	// Break up into WORDs
	WORD wCy4, wCy3, wCy2, wCy1, wL2, wL1;
	
	wCy4 = HIWORD(curResult.m_cur.Hi);
	wCy3 = LOWORD(curResult.m_cur.Hi);
	wCy2 = HIWORD(curResult.m_cur.Lo);
	wCy1 = LOWORD(curResult.m_cur.Lo);
	
	wL2 = HIWORD(nTempOp);
	wL1 = LOWORD(nTempOp);
	
	// Multiply each set of WORDs
	DWORD dwRes11, dwRes12, dwRes21, dwRes22;
	DWORD dwRes31, dwRes32, dwRes41;  // Don't need dwRes42
	
	dwRes11 = wCy1 * wL1;
	dwRes12 = wCy1 * wL2;
	dwRes21 = wCy2 * wL1;
	dwRes22 = wCy2 * wL2;
	
	dwRes31 = wCy3 * wL1;
	dwRes32 = wCy3 * wL2;
	dwRes41 = wCy4 * wL1;
	
	// Add up low order pieces
	dwRes11 += dwRes12<<16;
	curResult.m_cur.Lo = dwRes11 + (dwRes21<<16);
	curResult.m_cur.Hi = 0;
	
	// Check if carry required
	if (dwRes11 < dwRes12<<16)
		curResult.m_cur.Hi++;
	if ((DWORD)curResult.m_cur.Lo < dwRes11)
		curResult.m_cur.Hi++;
	
	// Add up the high order pieces
	curResult.m_cur.Hi += dwRes31 + (dwRes32<<16) + (dwRes41<<16) +
		dwRes22 + (dwRes12>>16) + (dwRes21>>16);
	
	// Compute result sign
	if ((m_cur.Hi ^ nOperand) & 0x80000000)
		curResult = -curResult;
	
	return curResult;
}

CxOleCurrency CxOleCurrency::operator/(long nOperand) const
{
	// If operand not Valid, just return
	if (!GetStatus() == valid)
		return *this;
	
	CxOleCurrency curTemp(m_cur);
	DWORD nTempOp;
	
	// Check for divide by 0
	if (nOperand == 0)
	{
		curTemp.SetStatus(invalid);
		
		// Set to maximum negative value
		curTemp.m_cur.Hi = 0x80000000;
		curTemp.m_cur.Lo = 0x00000000;
		
		return curTemp;
	}
	
	// Compute absolute values
	if (curTemp.m_cur.Hi < 0)
		curTemp = -curTemp;
	
	nTempOp = labs(nOperand);
	
	// Optimization - division is simple if Hi == 0
	if (curTemp.m_cur.Hi == 0x0000)
	{
		curTemp.m_cur.Lo = m_cur.Lo / nTempOp;
		
		// Compute result sign
		if ((m_cur.Hi ^ nOperand) & 0x80000000)
			curTemp = -curTemp;
		
		return curTemp;
	}
	
	// Now curTemp represents remainder
	CxOleCurrency curResult; // Initializes to zero
	CxOleCurrency curTempResult;
	CxOleCurrency curOperand;
	
	curOperand.m_cur.Lo = nTempOp;
	
	WORD wHiBitRem;
	WORD wScaleOp;
	
	// Quit if remainder can be truncated
	while (curTemp >= curOperand)
	{
		// Scale up and divide Hi portion
		HI_BIT(curTemp.m_cur.Hi, wHiBitRem);
		
		if (wHiBitRem != 0)
			wHiBitRem += 32;
		else
			HI_BIT(curTemp.m_cur.Lo, wHiBitRem);
		
		WORD wShift = (WORD)(64 - wHiBitRem);
		LSHIFT_UCUR(curTemp, wShift);
		
		// If Operand bigger than Hi it must be scaled
		wScaleOp = (WORD)((nTempOp > (DWORD)curTemp.m_cur.Hi) ? 1 : 0);
		
		// Perform synthetic division
		curTempResult.m_cur.Hi =
			(DWORD)curTemp.m_cur.Hi / (nTempOp >> wScaleOp);
		
		// Scale back to get correct result and remainder
		RSHIFT_UCUR(curTemp, wShift);
		wShift = (WORD)(wShift - wScaleOp);
		RSHIFT_UCUR(curTempResult, wShift);
		
		// Now calculate result and remainder
		curResult += curTempResult;
		curTemp -= curTempResult * nTempOp;
	}
	
	// Compute result sign
	if ((m_cur.Hi ^ nOperand) & 0x80000000)
		curResult = -curResult;
	
	return curResult;
}

void CxOleCurrency::SetCurrency(long nUnits, long nFractionalUnits)
{
	CxOleCurrency curUnits;              // Initializes to 0
	CxOleCurrency curFractionalUnits;    // Initializes to 0
	
	// Set temp currency value to Units (need to multiply by 10,000)
	curUnits.m_cur.Lo = (DWORD)labs(nUnits);
	curUnits = curUnits * 10000;
	if (nUnits < 0)
		curUnits = -curUnits;
	
	curFractionalUnits.m_cur.Lo = (DWORD)labs(nFractionalUnits);
	if (nFractionalUnits < 0)
		curFractionalUnits = -curFractionalUnits;
	
	// Now add together Units and FractionalUnits
	*this = curUnits + curFractionalUnits;
	
	SetStatus(valid);
}

BOOL CxOleCurrency::ParseCurrency(LPCTSTR lpszCurrency,
								  DWORD dwFlags,  LCID lcid)
{
	USES_CONVERSION;
	CxString strCurrency = lpszCurrency;
	
	SCODE sc;
	if ( FAILED(sc = VarCyFromStr((LPOLESTR)T2COLE(strCurrency),
		lcid, dwFlags, &m_cur)))
	{
		if (sc == DISP_E_TYPEMISMATCH)
		{
			// Can't convert string to CURRENCY, set 0 & invalid
			m_cur.Hi = 0x00000000;
			m_cur.Lo = 0x00000000;
			SetStatus(invalid);
			return FALSE;
		}
		else if (sc == DISP_E_OVERFLOW)
		{
			// Can't convert string to CURRENCY, set max neg & invalid
			m_cur.Hi = 0x80000000;
			m_cur.Lo = 0x00000000;
			SetStatus(invalid);
			return FALSE;
		}
		else
		{
			XTRACE(_T("\nCOleCurrency VarCyFromStr call failed.\n\t"));
			if (sc == E_OUTOFMEMORY)
				throw CxException( _T("CxOleCurrency"), _T("Memory Exception") );
			else
				throw CxException( _T("CxOleCurrency"), _T("ERROR") );
		}
	}
	
	SetStatus(valid);
	return TRUE;
}

CxString CxOleCurrency::Format(DWORD dwFlags, LCID lcid) const
{
	USES_CONVERSION;
	CxString strCur;
	
	// If null, return empty string
	if (GetStatus() == null)
		return strCur;
	
	// If invalid, return Currency resource string
	if (GetStatus() == invalid)
	{
		strCur = _T("Invalid currency");
		//XVERIFY(strCur.LoadString(AFX_IDS_INVALID_CURRENCY));
		return strCur;
	}
	
	CxOleVariant var;
	// Don't need to trap error. Should not fail due to type mismatch
	_CheckError(VarBstrFromCy(m_cur, lcid, dwFlags, &V_BSTR(&var)));
	var.vt = VT_BSTR;
	return OLE2CT(V_BSTR(&var));
}


// CxOleVariant
CxOleVariant::CxOleVariant()
{ _VariantInit(this); }
CxOleVariant::~CxOleVariant()
{ XVERIFY(::VariantClear(this) == 0); }
void CxOleVariant::Clear()
{ XVERIFY(::VariantClear(this) == 0); }
CxOleVariant::CxOleVariant(LPCTSTR lpszSrc)
{ vt = VT_EMPTY; *this = lpszSrc; }
CxOleVariant::CxOleVariant(CxString& strSrc)
{ vt = VT_EMPTY; *this = strSrc; }
CxOleVariant::CxOleVariant(BYTE nSrc)
{ vt = VT_UI1; bVal = nSrc; }
CxOleVariant::CxOleVariant(const CxOleCurrency& curSrc)
{ vt = VT_CY; cyVal = curSrc.m_cur; }
CxOleVariant::CxOleVariant(float fltSrc)
{ vt = VT_R4; fltVal = fltSrc; }
CxOleVariant::CxOleVariant(double dblSrc)
{ vt = VT_R8; dblVal = dblSrc; }
CxOleVariant::CxOleVariant(const CxOleDateTime& dateSrc)
{ vt = VT_DATE; date = dateSrc.m_dt; }
BOOL CxOleVariant::operator==(LPCVARIANT pSrc) const
{ return *this == *pSrc; }
CxOleVariant::operator LPVARIANT()
{ return this; }
CxOleVariant::operator LPCVARIANT() const
{ return this; }

// CxOleCurrency
CxOleCurrency::CxOleCurrency()
{ m_cur.Hi = 0; m_cur.Lo = 0; SetStatus(valid); }
CxOleCurrency::CxOleCurrency(CURRENCY cySrc)
{ m_cur = cySrc; SetStatus(valid); }
CxOleCurrency::CxOleCurrency(const CxOleCurrency& curSrc)
{ m_cur = curSrc.m_cur; m_status = curSrc.m_status; }
CxOleCurrency::CxOleCurrency(const VARIANT& varSrc)
{ *this = varSrc; }
CxOleCurrency::CurrencyStatus CxOleCurrency::GetStatus() const
{ return m_status; }
void CxOleCurrency::SetStatus(CurrencyStatus status)
{ m_status = status; }
const CxOleCurrency& CxOleCurrency::operator+=(const CxOleCurrency& cur)
{ *this = *this + cur; return *this; }
const CxOleCurrency& CxOleCurrency::operator-=(const CxOleCurrency& cur)
{ *this = *this - cur; return *this; }
const CxOleCurrency& CxOleCurrency::operator*=(long nOperand)
{ *this = *this * nOperand; return *this; }
const CxOleCurrency& CxOleCurrency::operator/=(long nOperand)
{ *this = *this / nOperand; return *this; }
BOOL CxOleCurrency::operator==(const CxOleCurrency& cur) const
{ return(m_status == cur.m_status && m_cur.Hi == cur.m_cur.Hi &&
		 m_cur.Lo == cur.m_cur.Lo); }
BOOL CxOleCurrency::operator!=(const CxOleCurrency& cur) const
{ return(m_status != cur.m_status || m_cur.Hi != cur.m_cur.Hi ||
		 m_cur.Lo != cur.m_cur.Lo); }
CxOleCurrency::operator CURRENCY() const
{ return m_cur; }

// CxOleDateTimeSpan
CxOleDateTimeSpan::CxOleDateTimeSpan()
{ m_span = 0; SetStatus(valid); }
CxOleDateTimeSpan::CxOleDateTimeSpan(double dblSpanSrc)
{ m_span = dblSpanSrc; SetStatus(valid); }
CxOleDateTimeSpan::CxOleDateTimeSpan(
									 const CxOleDateTimeSpan& dateSpanSrc)
{ m_span = dateSpanSrc.m_span; m_status = dateSpanSrc.m_status; }
CxOleDateTimeSpan::CxOleDateTimeSpan(
									 long lDays, int nHours, int nMins, int nSecs)
{ SetDateTimeSpan(lDays, nHours, nMins, nSecs); }
CxOleDateTimeSpan::DateTimeSpanStatus CxOleDateTimeSpan::GetStatus() const
{ return m_status; }
void CxOleDateTimeSpan::SetStatus(DateTimeSpanStatus status)
{ m_status = status; }
double CxOleDateTimeSpan::GetTotalDays() const
{ XASSERT(GetStatus() == valid); return m_span; }
double CxOleDateTimeSpan::GetTotalHours() const
{ XASSERT(GetStatus() == valid);
long lReturns = (long)(m_span * 24 + _OLE_DATETIME_HALFSECOND);
return lReturns;
}
double CxOleDateTimeSpan::GetTotalMinutes() const
{ XASSERT(GetStatus() == valid);
long lReturns = (long)(m_span * 24 * 60 + _OLE_DATETIME_HALFSECOND);
return lReturns;
}
double CxOleDateTimeSpan::GetTotalSeconds() const
{ XASSERT(GetStatus() == valid);
long lReturns = (long)(m_span * 24 * 60 * 60 + _OLE_DATETIME_HALFSECOND);
return lReturns;
}

long CxOleDateTimeSpan::GetDays() const
{ XASSERT(GetStatus() == valid); return (long)m_span; }
BOOL CxOleDateTimeSpan::operator==(
								   const CxOleDateTimeSpan& dateSpan) const
{ return (m_status == dateSpan.m_status &&
		  m_span == dateSpan.m_span); }
BOOL CxOleDateTimeSpan::operator!=(
								   const CxOleDateTimeSpan& dateSpan) const
{ return (m_status != dateSpan.m_status ||
		  m_span != dateSpan.m_span); }
BOOL CxOleDateTimeSpan::operator<(
								  const CxOleDateTimeSpan& dateSpan) const
{ XASSERT(GetStatus() == valid);
XASSERT(dateSpan.GetStatus() == valid);
return m_span < dateSpan.m_span; }
BOOL CxOleDateTimeSpan::operator>(
								  const CxOleDateTimeSpan& dateSpan) const
{ XASSERT(GetStatus() == valid);
XASSERT(dateSpan.GetStatus() == valid);
return m_span > dateSpan.m_span; }
BOOL CxOleDateTimeSpan::operator<=(
								   const CxOleDateTimeSpan& dateSpan) const
{ XASSERT(GetStatus() == valid);
XASSERT(dateSpan.GetStatus() == valid);
return m_span <= dateSpan.m_span; }
BOOL CxOleDateTimeSpan::operator>=(
								   const CxOleDateTimeSpan& dateSpan) const
{ XASSERT(GetStatus() == valid);
XASSERT(dateSpan.GetStatus() == valid);
return m_span >= dateSpan.m_span; }
const CxOleDateTimeSpan& CxOleDateTimeSpan::operator+=(
													   const CxOleDateTimeSpan dateSpan)
{ *this = *this + dateSpan; return *this; }
const CxOleDateTimeSpan& CxOleDateTimeSpan::operator-=(
													   const CxOleDateTimeSpan dateSpan)
{ *this = *this - dateSpan; return *this; }
CxOleDateTimeSpan CxOleDateTimeSpan::operator-() const
{ return -this->m_span; }
CxOleDateTimeSpan::operator double() const
{ return m_span; }

// CxOleDateTime
CxOleDateTime::CxOleDateTime()
{ m_dt = 0; SetStatus(valid); }
CxOleDateTime::CxOleDateTime(const CxOleDateTime& dateSrc)
{ m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; }
CxOleDateTime::CxOleDateTime(const VARIANT& varSrc)
{ *this = varSrc; }
CxOleDateTime::CxOleDateTime(DATE dtSrc)
{ m_dt = dtSrc; SetStatus(valid); }
CxOleDateTime::CxOleDateTime(time_t timeSrc)
{ *this = timeSrc; }
CxOleDateTime::CxOleDateTime(const SYSTEMTIME& systimeSrc)
{ *this = systimeSrc; }
CxOleDateTime::CxOleDateTime(const FILETIME& filetimeSrc)
{ *this = filetimeSrc; }
CxOleDateTime::CxOleDateTime(int nYear, int nMonth, int nDay,
							 int nHour, int nMin, int nSec)
{ SetDateTime(nYear, nMonth, nDay, nHour, nMin, nSec); }
const CxOleDateTime& CxOleDateTime::operator=(const CxOleDateTime& dateSrc)
{ m_dt = dateSrc.m_dt; m_status = dateSrc.m_status; return *this; }
CxOleDateTime::DateTimeStatus CxOleDateTime::GetStatus() const
{ return m_status; }
void CxOleDateTime::SetStatus(DateTimeStatus status)
{ m_status = status; }
BOOL CxOleDateTime::operator==(const CxOleDateTime& date) const
{ return (m_status == date.m_status && m_dt == date.m_dt); }
BOOL CxOleDateTime::operator!=(const CxOleDateTime& date) const
{ return (m_status != date.m_status || m_dt != date.m_dt); }
const CxOleDateTime& CxOleDateTime::operator+=(
											   const CxOleDateTimeSpan dateSpan)
{ *this = *this + dateSpan; return *this; }
const CxOleDateTime& CxOleDateTime::operator-=(
											   const CxOleDateTimeSpan dateSpan)
{ *this = *this - dateSpan; return *this; }
CxOleDateTime::operator DATE() const
{ return m_dt; }
int CxOleDateTime::SetDate(int nYear, int nMonth, int nDay)
{ return SetDateTime(nYear, nMonth, nDay, 0, 0, 0); }
int CxOleDateTime::SetTime(int nHour, int nMin, int nSec)
// Set date to zero date - 12/30/1899
{ return SetDateTime(1899, 12, 30, nHour, nMin, nSec); }