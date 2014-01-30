#ifndef __X_PING_UTIL_H__
#define __X_PING_UTIL_H__

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/export.h>

class XUTIL_API CxPingUtil  
{
public:
	struct PingReply
	{
//		in_addr Address;			// the IP address of the replier
		unsigned long RTT;			// Round Trip Time in milliseconds
		unsigned long Status;		// last ping result
		BOOL     bNetworkError;
	};
private:
	CxPingUtil() {}
	virtual ~CxPingUtil() {}

public:
	static BOOL Ping( LPCTSTR pszHostName, CxPingUtil::PingReply& pr, UCHAR nTTL = 10, DWORD dwTimeout = 5000, WORD wDataSize = 32, UCHAR nTOS = 0, BOOL bDontFragment = FALSE );
};


#endif