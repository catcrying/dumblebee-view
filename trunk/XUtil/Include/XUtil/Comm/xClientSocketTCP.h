// xClientSocketTCP.h: interface for the CxClientSocketTCP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIENTSOCKET_H__7BAD9B9C_E0E1_4C53_B518_237853E9DB2E__INCLUDED_)
#define AFX_CLIENTSOCKET_H__7BAD9B9C_E0E1_4C53_B518_237853E9DB2E__INCLUDED_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <XUtil/export.h>
#include <XUtil/Comm/xIoBuffer.h>
#include <XUtil/String/xString.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxClientSocketTCP;
typedef struct _ClientThreadContext
{
	HANDLE				hThread;
	SOCKET				hSocket;
	CxClientSocketTCP*	pThis;
	WSAEVENT			hNetEvent[2];
	CxIoBuffer			IoBuffer;
	BOOL				bConnectOK;
	unsigned int		nThreadId;
} ClientThreadContext;

class XUTIL_API CxClientSocketTCP
{
protected:
	WSAEVENT	m_hCleanupEvent;
	ClientThreadContext	m_ThreadContext;
	BOOL		m_bStarted;
	
	UINT		m_nPort;
	CxString	m_strIPAddress;

	int		m_nMaxRecvBufferSize;
	int		m_nIOBufferSize;
	
	BOOL InitializeSocket();
	BOOL CreateConnectedSocket();
	static unsigned int __stdcall WorkerThread( LPVOID lpParam );
	
	BOOL CreateWorkerThread();
	BOOL StopWorkerThread();
	BOOL CloseConnectedSocket();

	BOOL ConnectServerA( ClientThreadContext* pContext, LPCSTR lpszIP, int nPort );
	BOOL ConnectServerW( ClientThreadContext* pContext, LPCWSTR lpszIP, int nPort );
#ifdef _UNICODE
	#define ConnectServer ConnectServerW
#else
	#define ConnectServer ConnectServerA
#endif

	BOOL SendBuffer( ClientThreadContext* pContext, char *pOutBuffer, int nOutBufferSize );

	virtual void ProcessBuffer( ClientThreadContext* pContext, char* pReceiveBuf, int nRecvBytes ) = 0;
public:
	CxClientSocketTCP();
	virtual ~CxClientSocketTCP();

	CxString GetIP() const;

	const UINT GetPort() const;

	const int GetIOBufferSize() const;

	BOOL StartClientA( LPCSTR lpszIP, int nPort, int nIOBufferSize=15*1024*1024 );
	BOOL StartClientW( LPCWSTR lpszIP, int nPort, int nIOBufferSize=15*1024*1024 );
#ifdef _UNICODE
	#define StartClient StartClientW
#else
	#define StartClient StartClientA
#endif

	BOOL StopClient();

	virtual void OnPacketReceived( ClientThreadContext* pContext, CxIoBuffer* PacketBuffer ) {}
	virtual void OnStartClient() {}
	virtual void OnStopClient() {}
	virtual void OnConnectionError() {}
	virtual void OnAbnormalClose() {}
};

#endif // !defined(AFX_CLIENTSOCKET_H__7BAD9B9C_E0E1_4C53_B518_237853E9DB2E__INCLUDED_)
