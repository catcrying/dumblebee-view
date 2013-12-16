// xClientSocketTCP.cpp: implementation of the CxClientSocketTCP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XUtil/Comm/xClientSocketTCP.h>
#include <atlconv.h>
#include <process.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define xmalloc(s) ::HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, (s) )
#define xfree(p)   ::HeapFree( GetProcessHeap(), 0, (p) )

#define MAX_SEND_BUF_SIZE		(4096*1024)		// 4K
#define MAX_RECV_BUF_SIZE		(4096*1024)		// 4K

CxClientSocketTCP::CxClientSocketTCP()
{
	m_bStarted = FALSE;

	m_ThreadContext.hSocket = INVALID_SOCKET;
	m_ThreadContext.hThread = NULL;
	m_ThreadContext.pThis = this;
	m_ThreadContext.hNetEvent[0] = m_hCleanupEvent;
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;
	m_ThreadContext.IoBuffer.Clear();
	m_ThreadContext.bConnectOK = FALSE;
	m_hCleanupEvent = WSA_INVALID_EVENT;
}

CxClientSocketTCP::~CxClientSocketTCP()
{
}

CxString CxClientSocketTCP::GetIP() const
{
	return m_strIPAddress;
}

const UINT CxClientSocketTCP::GetPort() const
{
	return m_nPort; 
}

const int CxClientSocketTCP::GetIOBufferSize() const
{
	return m_nIOBufferSize;
}

BOOL CxClientSocketTCP::InitializeSocket()
{
	m_hCleanupEvent = WSA_INVALID_EVENT;

	m_hCleanupEvent = WSACreateEvent();
	if (m_hCleanupEvent == WSA_INVALID_EVENT)
	{
		XTRACE( _T("WSACreateEvent() failed: %d\r\n"), WSAGetLastError() );
		WSACleanup();
		return FALSE;
	}

	m_ThreadContext.hSocket = INVALID_SOCKET;
	m_ThreadContext.hThread = NULL;
	m_ThreadContext.pThis = this;
	m_ThreadContext.hNetEvent[0] = m_hCleanupEvent;
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;
	m_ThreadContext.IoBuffer.Create( m_nIOBufferSize );
	m_ThreadContext.IoBuffer.Clear();
	m_ThreadContext.bConnectOK = FALSE;

	return TRUE;
}

unsigned int __stdcall CxClientSocketTCP::WorkerThread( LPVOID lpParam )
{
	char * pRecvBuffer = NULL;
	ClientThreadContext* pContext = (ClientThreadContext*) lpParam;
	CxClientSocketTCP *pThis = (CxClientSocketTCP *)pContext->pThis;

	const int nRecvBufferSize = pThis->m_nMaxRecvBufferSize;

	if ( !(pRecvBuffer = (char *)xmalloc(nRecvBufferSize)) ) 
		return 0;

	int nEventIndex;
	WSANETWORKEVENTS hNetworkEvents;
	volatile BOOL bWork = TRUE;
	while ( bWork ) 
	{
		nEventIndex = WSAWaitForMultipleEvents( 2, pContext->hNetEvent, FALSE, 500, FALSE );

		nEventIndex -= WSA_WAIT_EVENT_0;

		for ( int ix=nEventIndex; ix<2; ix++ )
		{
			nEventIndex = WSAWaitForMultipleEvents( 1, &pContext->hNetEvent[ix], TRUE, 0, FALSE );

			if ( nEventIndex == WSA_WAIT_FAILED ) continue;
			else if ( nEventIndex == WSA_WAIT_TIMEOUT )
			{
			}
			else
			{
				if ( ix == 1 )
				{
					WSAEnumNetworkEvents( pContext->hSocket, pContext->hNetEvent[1], &hNetworkEvents );
					if ( hNetworkEvents.lNetworkEvents & FD_READ )
					{
						if ( hNetworkEvents.iErrorCode[FD_READ_BIT] != 0 )
						{
							XTRACE( _T("ThreadId: %X - Socket Read Error!\r\n"), pContext->nThreadId );
							break;
						}
						int nRecvBytes = recv( pContext->hSocket, pRecvBuffer, nRecvBufferSize, 0 );
						//XTRACE( _T("%d bytes received!\r\n"), nRecvBytes );

						if ( nRecvBytes < 0 )
						{
							XTRACE( _T("ThreadId: %X - Socket Read Error!\r\n"), pContext->nThreadId );
							break;
						}
						pThis->ProcessBuffer( pContext, pRecvBuffer, nRecvBytes );
					}

					if ( hNetworkEvents.lNetworkEvents & FD_CONNECT )
					{
						if ( hNetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0 )
						{
							if ( pContext->hNetEvent[1] != WSA_INVALID_EVENT )
							{
								WSACloseEvent( pContext->hNetEvent[1] );
								pContext->hNetEvent[1] = WSA_INVALID_EVENT;
							}

							if ( pContext->hSocket != INVALID_SOCKET )
							{
								char szBye[3] = { 0, };
								send( pContext->hSocket, szBye, 3, 0 );
								shutdown( pContext->hSocket, SD_BOTH );
								closesocket( pContext->hSocket );
								//XTRACE( "Closed socket handle %x\r\n", pContext->hSocket );

								pContext->hSocket = INVALID_SOCKET;
							}
							pContext->bConnectOK = FALSE;

							pThis->m_bStarted = FALSE;
							CloseHandle( pContext->hThread );
							pContext->hThread = NULL;
							pThis->OnConnectionError();
							bWork = FALSE;
							break;
						}
						else
						{
							pContext->bConnectOK = TRUE;
							pThis->OnStartClient();
						}
					}

					if ( hNetworkEvents.lNetworkEvents & FD_CLOSE )
					{
						if ( hNetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )	
						{
							XTRACE( _T("Abnormal closed\r\n") );
						}
						else
						{
							if ( pContext->hNetEvent[1] != WSA_INVALID_EVENT )
							{
								WSACloseEvent( pContext->hNetEvent[1] );
								pContext->hNetEvent[1] = WSA_INVALID_EVENT;
							}
						}

						if ( pContext->hSocket != INVALID_SOCKET)
						{
							char szBye[3] = { 0, };
							send( pContext->hSocket, szBye, 3, 0 );
							shutdown( pContext->hSocket, SD_BOTH );
							closesocket( pContext->hSocket );
							//XTRACE( "Closed socket handle %x\r\n", pContext->hSocket );

							pContext->hSocket = INVALID_SOCKET;
						}
						pContext->bConnectOK = FALSE;
						pThis->m_bStarted = FALSE;

						CloseHandle( pContext->hThread );
						pContext->hThread = NULL;
						pThis->OnAbnormalClose();
						bWork = FALSE;
						break;
					}
				}
				else
				{
					bWork = FALSE;
					break;
				}
			}
		}
	}

	if ( pRecvBuffer )
	{
		xfree( pRecvBuffer );
	}

	::_endthreadex(0);

	return 0;
}

BOOL CxClientSocketTCP::SendBuffer( ClientThreadContext* pContext, char *pOutBuffer, int nOutBufferSize ) 
{
	BOOL bRet = TRUE;
	char *pBuffer = pOutBuffer;
	int nTotalSend = 0;
	int nSend = 0;

	while ( nTotalSend < nOutBufferSize ) 
	{
		nSend = send( pContext->hSocket, pBuffer, nOutBufferSize - nTotalSend, 0);
		if ( nSend == SOCKET_ERROR ) 
		{
			XTRACE( _T("send(thread=%d) failed\r\n"), WSAGetLastError() );
			if ( WSAGetLastError() == WSAEWOULDBLOCK )
			{
				continue;
			}
			bRet = FALSE;
			break;
		} 
		else if ( nSend == 0 ) 
		{
			XTRACE( _T("connection closed\r\n") );
			bRet = FALSE;
			break;
		} 
		else 
		{
			nTotalSend += nSend;
			pBuffer += nSend;
		}
	}

	return bRet;
}

BOOL CxClientSocketTCP::CreateWorkerThread()
{
	m_ThreadContext.hThread = (HANDLE)::_beginthreadex( NULL, 0, WorkerThread, (LPVOID)&m_ThreadContext, CREATE_SUSPENDED, &m_ThreadContext.nThreadId );

	XTRACE( _T("ThreadId: %X\r\n"), m_ThreadContext.nThreadId );

	if ( m_ThreadContext.hThread == NULL )
	{
		XTRACE( _T("CreateThread failed: %d\n"), GetLastError() );
		return FALSE;
	}

	ResumeThread( m_ThreadContext.hThread );

	return TRUE;
}

BOOL CxClientSocketTCP::StartClientW( LPCWSTR lpszIP, int nPort, int nIOBufferSize /*=15*1024*1024*/ )
{
	USES_CONVERSION;
	m_strIPAddress = W2T((LPWSTR)lpszIP);
	m_nPort = nPort;
	m_nIOBufferSize = nIOBufferSize;
	InitializeSocket();

	if ( CreateConnectedSocket() )
	{
		if ( !CreateWorkerThread() )
		{
			StopClient();
			return FALSE;
		}
	}
	else
	{
		StopClient();
		return FALSE;
	}

	m_bStarted = TRUE;
	return TRUE;
}

BOOL CxClientSocketTCP::StartClientA( LPCSTR lpszIP, int nPort, int nIOBufferSize /*=15*1024*1024*/ )
{
	USES_CONVERSION;
	m_strIPAddress = A2T((LPSTR)lpszIP);
	m_nPort = nPort;
	m_nIOBufferSize = nIOBufferSize;
	InitializeSocket();

	if ( CreateConnectedSocket() )
	{
		if ( !CreateWorkerThread() )
		{
			StopClient();
			return FALSE;
		}
	}
	else
	{
		StopClient();
		return FALSE;
	}

	m_bStarted = TRUE;
	return TRUE;
}

BOOL CxClientSocketTCP::StopClient()
{
	StopWorkerThread();
	
	CloseConnectedSocket();

	if ( m_hCleanupEvent != WSA_INVALID_EVENT )
	{
		WSACloseEvent( m_hCleanupEvent );
		m_hCleanupEvent = WSA_INVALID_EVENT;
	}
	
	m_bStarted = FALSE;

	OnStopClient();

	return TRUE;
}

BOOL CxClientSocketTCP::CloseConnectedSocket()
{
	if ( m_ThreadContext.hNetEvent[1] != WSA_INVALID_EVENT )
	{
		WSACloseEvent( m_ThreadContext.hNetEvent[1] );
	}
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;

	if ( m_ThreadContext.hSocket != INVALID_SOCKET )
	{
		char szBye[3] = { 0, };
		send( m_ThreadContext.hSocket, szBye, 3, 0 );
		shutdown( m_ThreadContext.hSocket, SD_BOTH );
		closesocket( m_ThreadContext.hSocket );

		m_ThreadContext.hSocket = INVALID_SOCKET;
	}

	return TRUE;
}

BOOL CxClientSocketTCP::StopWorkerThread()
{
	if ( m_ThreadContext.hThread != NULL )
	{
		WSASetEvent( m_hCleanupEvent );
		if ( WAIT_OBJECT_0 != WaitForSingleObject( m_ThreadContext.hThread, 3000 ) )
		{
			TerminateThread( m_ThreadContext.hThread, 0 );
			XTRACE( _T("TerminateThread: %X\r\n"), m_ThreadContext.hThread );
		}
		CloseHandle( m_ThreadContext.hThread );
		m_ThreadContext.hThread = NULL;
	}

	if ( m_hCleanupEvent != WSA_INVALID_EVENT )
		WSACloseEvent( m_hCleanupEvent );
	m_hCleanupEvent = WSA_INVALID_EVENT;

	return TRUE;
}

BOOL CxClientSocketTCP::ConnectServerW( ClientThreadContext* pContext, LPCWSTR lpszIP, int nPort )
{
	USES_CONVERSION;
	return ConnectServerA(pContext, W2A(lpszIP), nPort);
}

BOOL CxClientSocketTCP::ConnectServerA( ClientThreadContext* pContext, LPCSTR lpszIP, int nPort )
{
	BOOL bRet = TRUE;
	int nRet =  0;

	SOCKADDR_IN addrin;

	const char *pAddress = lpszIP;
	
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons(nPort);
	addrin.sin_addr.s_addr = inet_addr(pAddress);

	pContext->hSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if ( pContext->hSocket == INVALID_SOCKET ) 
	{
		XTRACE( _T("socket() failed: %d\r\n"), WSAGetLastError() );
		bRet = FALSE;
	}

	int nMaxSendBufferSize = MAX_SEND_BUF_SIZE;
	int nMaxRecvBufferSize = MAX_RECV_BUF_SIZE;
	nRet = ::setsockopt( pContext->hSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nMaxSendBufferSize, sizeof(int) );
	nRet = ::setsockopt( pContext->hSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nMaxRecvBufferSize, sizeof(int) );

	int nLen = sizeof(int);
	nRet = ::getsockopt( pContext->hSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nMaxSendBufferSize, &nLen );
	nLen = sizeof(int);
	nRet = ::getsockopt( pContext->hSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nMaxRecvBufferSize, &nLen );

	m_nMaxRecvBufferSize = nMaxRecvBufferSize;

	XTRACE( _T("Send Buffer Size: %dbytes, Receive Buffer Size: %dbytes\r\n"), nMaxSendBufferSize, nMaxRecvBufferSize );

	pContext->hNetEvent[1] = WSACreateEvent();
	if ( WSAEventSelect( pContext->hSocket, pContext->hNetEvent[1], FD_READ|FD_CLOSE|FD_CONNECT ) == SOCKET_ERROR )
	{
		XTRACE( _T("WSAEventSelect() error\r\n") );
		return FALSE;
	}

	if ( bRet != FALSE ) 
	{
		nRet = ::connect(pContext->hSocket, reinterpret_cast<struct sockaddr *>(&addrin), sizeof(SOCKADDR_IN));
		if ( nRet == SOCKET_ERROR ) 
		{
			if ( WSAGetLastError() == WSAEWOULDBLOCK )
				return TRUE;
			XTRACE( _T("connect(0x%x) failed: %d\r\n"), pContext->hSocket, WSAGetLastError());
			bRet = FALSE;
		} 
		else
		{
			XTRACE( _T("connected(0x%x)\r\n"), pContext->hSocket );
		}

	}

	return bRet;
}

BOOL CxClientSocketTCP::CreateConnectedSocket() 
{
	if ( !ConnectServer( &m_ThreadContext, m_strIPAddress, m_nPort ) )
	{
		XTRACE( _T("CONNECT ERROR: Server: %s, Port: %d\r\n"), m_strIPAddress, m_nPort );
		return FALSE;
	}
	return TRUE;
}
