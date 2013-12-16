// xServerSocketTCP.cpp: implementation of the CxServerSocketTCP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XUtil/Comm/xServerSocketTCP.h>

#include <process.h>

#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p)   HeapFree(GetProcessHeap(),0,(p))

#define MAX_SEND_BUF_SIZE		(4096*1024)		// 4K
#define MAX_RECV_BUF_SIZE		(4096*1024)		// 4K

CxServerSocketTCP::CxServerSocketTCP()
{
	m_hCleanupEvent = WSA_INVALID_EVENT;
 
	m_ThreadContext.hSocket = INVALID_SOCKET;
	m_ThreadContext.hThread = NULL;
	m_ThreadContext.pThis = this;
	m_ThreadContext.hNetEvent[0] = m_hCleanupEvent;
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;
	m_ThreadContext.IoBuffer.Clear();
	m_ThreadContext.bListen = TRUE;
	m_ThreadContext.nClientPort = 0;
	m_ThreadContext.strClientIP.Empty();	

	m_bStarted = FALSE;	

	m_nMaxSendBufferSize = 0;
	m_nMaxRecvBufferSize = 0;
	m_nIOBufferSize = 0;
}

CxServerSocketTCP::~CxServerSocketTCP()
{
	StopServer();
}

const int CxServerSocketTCP::GetIOBufferSize() const
{
	return m_nIOBufferSize;
}

BOOL CxServerSocketTCP::InitializeSocket()
{
	m_hCleanupEvent = WSACreateEvent();
	if ( m_hCleanupEvent == WSA_INVALID_EVENT ) 
	{
		XTRACE( _T("WSACreateEvent() failed: %d\r\n"), WSAGetLastError() );
		return FALSE;
	}

	m_ThreadContext.hSocket = INVALID_SOCKET;
	m_ThreadContext.hThread = NULL;
	m_ThreadContext.pThis = this;
	m_ThreadContext.hNetEvent[0] = m_hCleanupEvent;
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;
	m_ThreadContext.IoBuffer.Create( m_nIOBufferSize );
	m_ThreadContext.IoBuffer.Clear();
	m_ThreadContext.bListen = TRUE;
	m_ThreadContext.nClientPort = 0;
	m_ThreadContext.strClientIP.Empty();

	XTRACE( _T("Socket Initialized...\r\n") );

	return TRUE;
}

BOOL CxServerSocketTCP::CreateServer( ServerThreadContext* pContext )
{
	int nRet =  0;

	SOCKADDR_IN addrin; 
	
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons( m_nPort );
	addrin.sin_addr.s_addr = htonl(INADDR_ANY);

	pContext->hSocket = ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED); 
	if ( pContext->hSocket == INVALID_SOCKET ) 
	{
		XTRACE( _T("socket() failed: %d\r\n"), WSAGetLastError() );
		return FALSE;
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

	nRet = ::bind(pContext->hSocket, reinterpret_cast<struct sockaddr *>(&addrin), sizeof(SOCKADDR_IN));

	if ( nRet == SOCKET_ERROR ) 
	{
		XTRACE( _T("bind(0x%x) failed: %d\r\n"), pContext->hSocket, WSAGetLastError());
		return FALSE;
	} 
	else
	{
		XTRACE( _T("bind(0x%x)\r\n"), pContext->hSocket );
	}

	nRet = ::listen( pContext->hSocket, 5 );

	if ( nRet == SOCKET_ERROR ) 
	{
		XTRACE( _T("listen(0x%x) failed: %d\r\n"), pContext->hSocket, WSAGetLastError());
		return FALSE;
	} 
	else
	{
		XTRACE( _T("listen(0x%x)\r\n"), pContext->hSocket );
	}

	XTRACE( _T("Create Server: OK!\r\n") );

	return TRUE;
}

BOOL CxServerSocketTCP::CreateSocketServer() 
{
	if ( !CreateServer( &m_ThreadContext ) )
	{
		XTRACE( _T("CONNECT ERROR\r\n") );
		return FALSE;
	}

	m_ThreadContext.hNetEvent[1] = WSACreateEvent();
	if ( WSAEventSelect( m_ThreadContext.hSocket, m_ThreadContext.hNetEvent[1], FD_READ|FD_CLOSE|FD_ACCEPT ) == SOCKET_ERROR )
	{
		XTRACE( _T("WSAEventSelect() error\r\n") );

		return FALSE;
	}

	XTRACE( _T("Create Socket Server: OK!\r\n") );
	return TRUE;
}

unsigned int __stdcall CxServerSocketTCP::WorkerThread( LPVOID lpParam )
{
	char * pRecvBuffer = NULL;
	ServerThreadContext* pContext = (ServerThreadContext*) lpParam;
	CxServerSocketTCP *pThis = (CxServerSocketTCP *)pContext->pThis;

	XTRACE( _T("Starting thread 0x%x\r\n"), pContext->hThread );

	if ( !pContext->bListen )
		pThis->OnClientConnected( pContext, pContext->strClientIP, pContext->nClientPort );

	const int nRecvBufferSize = pThis->m_nMaxRecvBufferSize;
	if ( !(pRecvBuffer = (char *)xmalloc(nRecvBufferSize)) ) 
		return 0;

	int nEventIndex;
	WSANETWORKEVENTS hNetworkEvents;
	BOOL bWork = TRUE;
	while ( bWork ) 
	{
		nEventIndex = WSAWaitForMultipleEvents( 2, pContext->hNetEvent, FALSE, 500, FALSE );

		nEventIndex -= WSA_WAIT_EVENT_0;

		for ( int ix=nEventIndex ; ix<2 ; ix++ )
		{
			nEventIndex = WSAWaitForMultipleEvents( 1, &pContext->hNetEvent[ix], TRUE, 0, FALSE );

			if ( (nEventIndex == WSA_WAIT_FAILED ) ) continue;
			else if ( (nEventIndex==WSA_WAIT_TIMEOUT) )
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
							XTRACE( _T("Socket Read Error!\r\n") );
							break;
						}
						int nRecvBytes = recv( pContext->hSocket, pRecvBuffer, nRecvBufferSize, 0 );

						if ( nRecvBytes > 0 )
						{
							//XTRACE( _T("%d bytes received!\r\n"), nRecvBytes );
							pThis->ProcessBuffer( pContext, pRecvBuffer, nRecvBytes );
						}
						else
						{
							char szBye[3] = { 0, };
							send( pContext->hSocket, szBye, 3, 0 );
							shutdown( pContext->hSocket, SD_BOTH );
							closesocket( pContext->hSocket );
							XTRACE( _T("Closed socket handle %x\r\n"), pContext->hSocket );
							
							pContext->hSocket = INVALID_SOCKET;

							if ( pContext->bListen )
								pThis->OnStopServer();
							else
							{
								pThis->OnClientDisconnected( pContext->strClientIP, pContext->nClientPort );
								bWork = FALSE;
							}

						}
					}

					if ( hNetworkEvents.lNetworkEvents & FD_ACCEPT )
					{
						if ( hNetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0 )
						{
							XTRACE( _T("Accept Error!\r\n") );
						}

						SOCKADDR_IN addrinClient;
						int nAddrSize = sizeof(SOCKADDR_IN);
						SOCKET hClientSocket = ::accept( pContext->hSocket, reinterpret_cast<struct sockaddr *>(&addrinClient), &nAddrSize );

						if ( hClientSocket == INVALID_SOCKET )
						{
							XTRACE( _T("accept() failed: %d\r\n"), WSAGetLastError() );
							break;
						}

						int nRet;
						int nMaxSendBufferSize = MAX_SEND_BUF_SIZE;
						int nMaxRecvBufferSize = MAX_RECV_BUF_SIZE;
						nRet = ::setsockopt( hClientSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nMaxSendBufferSize, sizeof(int) );
						nRet = ::setsockopt( hClientSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nMaxRecvBufferSize, sizeof(int) );

						int nLen = sizeof(int);
						nRet = ::getsockopt( hClientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&nMaxSendBufferSize, &nLen );
						nLen = sizeof(int);
						nRet = ::getsockopt( hClientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&nMaxRecvBufferSize, &nLen );

						CxString strClientIP;
						int nPort;

						TCHAR szClientAddr[50]={0,};
						DWORD dwSize = sizeof(szClientAddr);
						SOCKADDR* pClientSockAddr = (SOCKADDR*)&addrinClient;
						CxString strClientSockAddr;
						if ( WSAAddressToString( pClientSockAddr, sizeof(SOCKADDR), NULL, szClientAddr, &dwSize) != SOCKET_ERROR )
						{
							strClientSockAddr = szClientAddr;
						}

						int nColonPos = strClientSockAddr.ReverseFind( _T(':') );
						strClientIP = strClientSockAddr.Left( nColonPos );
						strClientSockAddr.Delete( 0, nColonPos+1 );

						nPort = _tcstol(strClientSockAddr, NULL, 10);

						ServerThreadContext* pThreadContext = new ServerThreadContext;

						pThreadContext->hSocket = hClientSocket;
						pThreadContext->hThread = NULL;
						pThreadContext->pThis = pThis;
						pThreadContext->hNetEvent[0] = pThis->m_hCleanupEvent;
						pThreadContext->hNetEvent[1] = WSA_INVALID_EVENT;
						pThreadContext->IoBuffer.Create( pThis->GetIOBufferSize() );
						pThreadContext->IoBuffer.Clear();
						pThreadContext->bListen = FALSE;
						pThreadContext->strClientIP = strClientIP;
						pThreadContext->nClientPort = nPort;
						pThreadContext->dwUserData = 0;
						
						unsigned int nThreadID = 0;
						pThreadContext->hThread = (HANDLE)::_beginthreadex( NULL, 0, WorkerThread, pThreadContext, 0, &nThreadID );


						pThreadContext->hNetEvent[1] = WSACreateEvent();
						if ( WSAEventSelect( pThreadContext->hSocket, pThreadContext->hNetEvent[1], FD_READ|FD_CLOSE ) == SOCKET_ERROR )
						{
							XTRACE( _T("WSAEventSelect() error\r\n") );
							break;
						}
						
						if ( pThreadContext->hThread == NULL )
						{
							delete pThreadContext;
							break;
						}

						CloseHandle( pThreadContext->hThread );
					}

					if ( hNetworkEvents.lNetworkEvents & FD_CLOSE ) // 연결 종료
					{
						if ( hNetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0 )	
						{
							XTRACE( _T("Abnormal closed...!!!\r\n") );
						}
						else
						{
							WSACloseEvent( pContext->hNetEvent[1] );
							pContext->hNetEvent[1] = WSA_INVALID_EVENT;
						}

						char szBye[3] = { 0, };
						send( pContext->hSocket, szBye, 3, 0 );
						shutdown( pContext->hSocket, SD_BOTH );
						closesocket( pContext->hSocket );
						XTRACE( _T("Closed socket handle %x\r\n"), pContext->hSocket );

						pContext->hSocket = INVALID_SOCKET;

						if ( pContext->bListen )
							pThis->OnStopServer();
						else
						{
							pThis->OnClientDisconnected( pContext->strClientIP, pContext->nClientPort );
							bWork = FALSE;
						}
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
		xfree(pRecvBuffer);

	if ( !pContext->bListen )
	{
		delete pContext;
	}

	_endthreadex(0);

	return 0;
}

BOOL CxServerSocketTCP::SendBuffer( SOCKET hSocket, char *pOutBuffer, int nOutBufferSize ) 
{
	BOOL bRet = TRUE;
	char *pBuffer = pOutBuffer;
	int nTotalSend = 0;
	int nSend = 0;

	while ( nTotalSend < nOutBufferSize ) 
	{
		nSend = send( hSocket, pBuffer, nOutBufferSize - nTotalSend, 0);
		if ( nSend == SOCKET_ERROR ) 
		{
			XTRACE( _T("send failed: %d\r\n"), WSAGetLastError() );
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


BOOL CxServerSocketTCP::StartServer( int nPort, int nIOBufferSize /*=15*1024*1024*/ )
{
	m_nPort = nPort;
	m_nIOBufferSize = nIOBufferSize;
	InitializeSocket();

	if ( !CreateSocketServer() )
		return FALSE;
	
	unsigned int nThreadID = 0;
	m_ThreadContext.hThread = (HANDLE)::_beginthreadex( NULL, 0, WorkerThread, &m_ThreadContext, 0, &nThreadID );

	if ( m_ThreadContext.hThread == NULL )
	{
		return FALSE;
	}

	OnStartServer();
	m_bStarted = TRUE;
	return TRUE;
}

BOOL CxServerSocketTCP::DisconnectConnection( ServerThreadContext* pContext )
{
	if ( pContext->hSocket != INVALID_SOCKET )
	{
		char szBye[3] = { 0, };
		send( pContext->hSocket, szBye, 3, 0 );
		shutdown( pContext->hSocket, SD_BOTH );
		closesocket( pContext->hSocket );
		pContext->hSocket = INVALID_SOCKET;
	}
	return TRUE;
}

BOOL CxServerSocketTCP::StopServer()
{
	if ( m_ThreadContext.hThread != NULL )
	{
		WSASetEvent( m_hCleanupEvent );
		if ( WAIT_OBJECT_0 != ::WaitForSingleObject( m_ThreadContext.hThread, 3000 ) )
		{
			::TerminateThread( m_ThreadContext.hThread, 0 );
		}
		::CloseHandle( m_ThreadContext.hThread );
		m_ThreadContext.hThread = NULL;
	}

	if ( m_hCleanupEvent != WSA_INVALID_EVENT )
		::WSACloseEvent( m_hCleanupEvent );
	m_hCleanupEvent = WSA_INVALID_EVENT;

	if ( m_ThreadContext.hNetEvent[1] != WSA_INVALID_EVENT )
		WSACloseEvent( m_ThreadContext.hNetEvent[1] );
	m_ThreadContext.hNetEvent[1] = WSA_INVALID_EVENT;

	if ( m_ThreadContext.hSocket != INVALID_SOCKET )
	{
		char szBye[3] = { 0, };
		send( m_ThreadContext.hSocket, szBye, 3, 0 );
		shutdown( m_ThreadContext.hSocket, SD_BOTH );
		closesocket( m_ThreadContext.hSocket );
		m_ThreadContext.hSocket = INVALID_SOCKET;
	}

	m_bStarted = FALSE;

	OnStopServer();

	return TRUE;
}
