/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */
 
#include "stdafx.h"
#include <tchar.h>
#include <XUtil/Comm/xPingUtil.h>
#include <XUtil/DebugSupport/xDebug.h>

#include <winsock.h>
#include <atlconv.h>

#pragma comment( lib, "ws2_32.lib" )

class _CPING_ICMP
{
public:
	typedef unsigned long IPAddr;     // An IP address.
	
	typedef struct tagIP_OPTION_INFORMATION 
	{
		unsigned char      Ttl;              // Time To Live
		unsigned char      Tos;              // Type Of Service
		unsigned char      Flags;            // IP header flags
		unsigned char      OptionsSize;      // Size in bytes of options data
		unsigned char FAR *OptionsData;      // Pointer to options data
	} IP_OPTION_INFORMATION;
	
	typedef struct tagICMP_ECHO_REPLY 
	{
		IPAddr         Address;       // Replying address
		unsigned long  Status;        // Reply IP_STATUS
		unsigned long  RoundTripTime; // RTT in milliseconds
		unsigned short DataSize;      // Reply data size in bytes
		unsigned short Reserved;      // Reserved for system use
		void FAR       *Data;         // Pointer to the reply data
		IP_OPTION_INFORMATION Options;       // Reply options
	} ICMP_ECHO_REPLY;
	
#define IP_FLAG_DF 0x2
	
	typedef IP_OPTION_INFORMATION FAR* LPIP_OPTION_INFORMATION;
	typedef ICMP_ECHO_REPLY FAR* LPICMP_ECHO_REPLY;
	
	_CPING_ICMP();
	~_CPING_ICMP();
	
protected:
	typedef HANDLE (WINAPI ICMPCREATEFILE)(void);
	typedef ICMPCREATEFILE* LPICMPCREATEFILE;
	typedef BOOL (WINAPI ICMPCLOSEHANDLE)(HANDLE);
	typedef ICMPCLOSEHANDLE* LPICMPCLOSEHANDLE;
	typedef DWORD (WINAPI ICMPSENDECHO)(HANDLE, IPAddr, LPVOID, WORD, LPIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);
	typedef ICMPSENDECHO* LPICMPSENDECHO;
	
	HINSTANCE         m_hIcmp;         //HINSTANCE of Iphlapi.DLL or as a fall back ICMP.DLL
	LPICMPCREATEFILE  m_pIcmpCreateFile;
	LPICMPSENDECHO    m_pIcmpSendEcho;
	LPICMPCLOSEHANDLE m_pIcmpCloseHandle;
	
	friend class CxPingUtil;
};

_CPING_ICMP::_CPING_ICMP()
{
	//First try the IP Helper library
	m_hIcmp = LoadLibrary(_T("Iphlpapi.dll"));
	if (m_hIcmp)
	{
		//Retrieve pointers to the functions in the ICMP dll
		m_pIcmpCreateFile = (LPICMPCREATEFILE) GetProcAddress(m_hIcmp,"IcmpCreateFile");
		m_pIcmpSendEcho = (LPICMPSENDECHO) GetProcAddress(m_hIcmp,"IcmpSendEcho" );
		m_pIcmpCloseHandle = (LPICMPCLOSEHANDLE) GetProcAddress(m_hIcmp,"IcmpCloseHandle");
		
		if (m_pIcmpCreateFile == NULL || m_pIcmpSendEcho == NULL ||	m_pIcmpCloseHandle == NULL)
		{
			XTRACE(_T("_CPING_ICMP::_CPING_ICMP, Could not find the required functions in the Iphlpapi dll, will try ICMP.dll\n"));
			
			FreeLibrary(m_hIcmp);
			m_hIcmp = NULL;
		}
	}
	
	if (m_hIcmp == NULL)
	{
		XTRACE(_T("_CPING_ICMP::_CPING_ICMP, Falling back to trying ICMP DLL\n"));
		
		//If not fall back to using the ICMP library
		m_hIcmp = LoadLibrary(_T("ICMP.DLL"));
		if (m_hIcmp)
		{
			//Retrieve pointers to the functions in the ICMP dll
			m_pIcmpCreateFile = (LPICMPCREATEFILE) GetProcAddress(m_hIcmp,"IcmpCreateFile");
			m_pIcmpSendEcho = (LPICMPSENDECHO) GetProcAddress(m_hIcmp,"IcmpSendEcho" );
			m_pIcmpCloseHandle = (LPICMPCLOSEHANDLE) GetProcAddress(m_hIcmp,"IcmpCloseHandle");
			
			if (m_pIcmpCreateFile == NULL || m_pIcmpSendEcho == NULL ||	m_pIcmpCloseHandle == NULL)
			{
				XTRACE(_T("_CPING_ICMP::_CPING_ICMP, Could not find the required functions in the ICMP.dll\n"));
				
				FreeLibrary(m_hIcmp);
				m_hIcmp = NULL;
			}
		}
		else
			XTRACE(_T("_CPING_ICMP::_CPING_ICMP, Could not load up the ICMP dll\n"));
	}
}

_CPING_ICMP::~_CPING_ICMP()
{
	if (m_hIcmp)
	{
		FreeLibrary(m_hIcmp);
		m_hIcmp = NULL;
	}
}

_CPING_ICMP _PingData;

BOOL CxPingUtil::Ping( LPCTSTR pszHostName, CxPingUtil::PingReply& pr, UCHAR nTTL /*= 10*/, DWORD dwTimeout /*= 5000*/, WORD wDataSize /*= 32*/, UCHAR nTOS /*= 0*/, BOOL bDontFragment /*= FALSE*/ )
{
	//For correct operation of the T2A macro, see TN059
	USES_CONVERSION;

	ZeroMemory( &pr, sizeof(CxPingUtil::PingReply) );
	
	//Make sure ICMP dll is available
	if (_PingData.m_hIcmp == NULL)
	{
		XTRACE(_T("CPing::PingUsingICMP, ICMP dll is not available\n"));
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
	}
	
	LPSTR lpszAscii = T2A((LPTSTR) pszHostName);
	//Convert from dotted notation if required
	unsigned long	addr = inet_addr(lpszAscii);
	if (addr == INADDR_NONE)
	{
		//Not a dotted address, then do a lookup of the name
		hostent* hp = gethostbyname(lpszAscii);
		if (hp)
			memcpy(&addr, hp->h_addr, hp->h_length);
		else
		{
			XTRACE(_T("CPing::PingUsingICMP, Could not resolve the host name %s\n"), pszHostName);
			return FALSE;
		}
	}
	
	//Create the ICMP handle
	XASSERT(_PingData.m_pIcmpCreateFile);
	HANDLE hIP = _PingData.m_pIcmpCreateFile();
	if (hIP == INVALID_HANDLE_VALUE)
	{
		XTRACE(_T("CPing::PingUsingICMP, Could not get a valid ICMP handle\n"));
		return FALSE;
	}
	
	//Set up the option info structure
	_CPING_ICMP::IP_OPTION_INFORMATION OptionInfo;
	memset(&OptionInfo, 0, sizeof(_CPING_ICMP::IP_OPTION_INFORMATION));
	OptionInfo.Ttl = nTTL;
	OptionInfo.Tos = nTOS;
	if (bDontFragment)
		OptionInfo.Flags = IP_FLAG_DF;
	
	//Set up the data which will be sent
	unsigned char* pBuf = new unsigned char[wDataSize];
	memset(pBuf, 'E', wDataSize);
	
	//Do the actual Ping
	DWORD dwReplySize = sizeof(_CPING_ICMP::ICMP_ECHO_REPLY) + max(wDataSize, 8);
	unsigned char* pReply = new unsigned char[dwReplySize];
	_CPING_ICMP::ICMP_ECHO_REPLY* pEchoReply = (_CPING_ICMP::ICMP_ECHO_REPLY*) pReply;
	XASSERT(_PingData.m_pIcmpSendEcho);
	DWORD nRecvPackets = _PingData.m_pIcmpSendEcho(hIP, addr, pBuf, wDataSize, &OptionInfo, pReply, dwReplySize, dwTimeout);
	
	//Check we got the packet back
	BOOL bSuccess = (nRecvPackets == 1);
	
	//Check the data we got back is what was sent
	if (bSuccess)
	{
		char* pReplyData = (char*) pEchoReply->Data;
		for (int i=0; i<pEchoReply->DataSize && bSuccess; i++)
			bSuccess = (pReplyData[i] == 'E');
		
		if (!bSuccess)
		{
			XTRACE(_T("CPing::PingUsingICMP, Could not get a valid ICMP response\n"));
			SetLastError(ERROR_UNEXP_NET_ERR);
		}
	}
	
	//Close the ICMP handle
	XASSERT(_PingData.m_pIcmpCloseHandle);
	_PingData.m_pIcmpCloseHandle(hIP);
	
	if (bSuccess)
	{
		//Ping was successful, copy over the pertinent info
		//into the return structure
//		pr.Address.S_un.S_addr = pEchoReply->Address;
		pr.RTT = pEchoReply->RoundTripTime;
		pr.Status = pEchoReply->Status;
		pr.bNetworkError = FALSE;
	}
	else
	{
		pr.RTT = 0xffffffff;
		pr.bNetworkError = TRUE;
	}
	
	//Free up the memory we allocated
	delete [] pBuf;
	delete [] pReply;
	
	//return the status
	return bSuccess; 
}