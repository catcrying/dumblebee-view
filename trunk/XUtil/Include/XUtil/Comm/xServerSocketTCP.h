// xServerSocketTCP.h: interface for the CxServerSocketTCP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SERVERSOCKET_H__A7807DDC_A8EE_4DAE_BA04_14D5A99A87CA__INCLUDED_)
#define AFX_SERVERSOCKET_H__A7807DDC_A8EE_4DAE_BA04_14D5A99A87CA__INCLUDED_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <XUtil/Comm/xIoBuffer.h>
#include <XUtil/String/xString.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CxServerSocketTCP;
typedef struct _ServerThreadContext
{
	HANDLE				hThread;
	SOCKET				hSocket;
	CxServerSocketTCP*	pThis;
	WSAEVENT			hNetEvent[2];
	CxIoBuffer			IoBuffer;
	BOOL				bListen;
	CxString			strClientIP;
	int					nClientPort;
	DWORD				dwUserData;
} ServerThreadContext;

class XUTIL_API CxServerSocketTCP  
{
protected:
	WSAEVENT			m_hCleanupEvent;
	ServerThreadContext	m_ThreadContext;
	UINT	m_nPort;
	BOOL    m_bStarted;
	
	int		m_nMaxSendBufferSize;
	int		m_nMaxRecvBufferSize;
	int		m_nIOBufferSize;

	BOOL InitializeSocket();
	
	BOOL CreateSocketServer();
	BOOL CreateServer( ServerThreadContext* pContext );

	static unsigned int __stdcall WorkerThread( LPVOID lpParam );

	virtual void ProcessBuffer( ServerThreadContext* pContext, char* pReceiveBuf, int nRecvBytes ) = 0;
	BOOL SendBuffer( SOCKET hSocket, char *pOutBuffer, int nOutBufferSize );

public:
	CxServerSocketTCP();
	virtual ~CxServerSocketTCP();

	BOOL StartServer( int nPort, int nIOBufferSize=15*1024*1024 );	// 4*1024*1024, 15*1024*1024
	BOOL StopServer();

	const int GetIOBufferSize() const;

	virtual void OnStopServer() {}
	virtual void OnStartServer() {}
	virtual void OnPacketReceived( ServerThreadContext* pContext, CxIoBuffer* PacketBuffer ) {}

	virtual void OnClientConnected( ServerThreadContext* pContext, CxString strClientIP, int nPort ) {}
	virtual void OnClientDisconnected( CxString strClientIP, int nPort ) {}

	BOOL DisconnectConnection( ServerThreadContext* pContext );
};

#endif // !defined(AFX_SERVERSOCKET_H__A7807DDC_A8EE_4DAE_BA04_14D5A99A87CA__INCLUDED_)
