// xSerialComm.cpp: implementation of the CxSerialComm class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <tchar.h>
#include <XUtil/Comm/xSerialComm.h>
#include <XUtil/DebugSupport/xDebug.h>
#include <XUtil/String/xString.h>
#include <process.h>

#define WM_XCOMM_CLOSE    0x0380
#define WM_XCOMM_RECEIVE  0x0381
#define WM_XCOMM_DISCONNECT_DEVICE 0x0382

#define MAX_BUF_SIZE   8192


class CxCommWorkGroup
{
public:
	CxCommWorkGroup()
		{ m_hWnd = NULL; }
	~CxCommWorkGroup()
		{ DestroyCommWindow(); }

private:
	HWND m_hWnd;
	static const TCHAR c_szWindowName[];

public:
	static BOOL RegisterClassWnd( HINSTANCE hInstance )
	{
   		WNDCLASS wc;
		wc.lpszClassName = CxCommWorkGroup::c_szWindowName;
		XASSERT( hInstance );
		wc.hInstance = hInstance;
		wc.lpfnWndProc = CxCommWorkGroup::XCOMMWGProc;
		wc.hCursor = NULL;
		wc.hIcon = NULL;
		wc.lpszMenuName = NULL;
		wc.hbrBackground = NULL;
		wc.style = CS_GLOBALCLASS;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		WNDCLASS wndcls;
		if ( ::GetClassInfo(wc.hInstance, wc.lpszClassName, &wndcls) )
		{
			if ( wndcls.style != wc.style )
			{
				XASSERT(FALSE);	// a other class exists that has same class name
				return FALSE;
			}
		}
		else return ::RegisterClass(&wc);

		return TRUE;
	}
	BOOL CreateCommWindow()
	{
		if ( m_hWnd ) return TRUE;
		m_hWnd = ::CreateWindowEx(0, CxCommWorkGroup::c_szWindowName,
			NULL, WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
		if ( m_hWnd == NULL )
		{
			XASSERT(FALSE);
			return FALSE;
		}
		::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
		return TRUE;
	}
	BOOL DestroyCommWindow()
	{
		if ( m_hWnd )
		{
			BOOL bRet = ::DestroyWindow(m_hWnd);
			m_hWnd = NULL;
			return bRet;
		}
		return TRUE;
	}
	BOOL PostMessage( CxSerialComm * pComm, UINT uMsg, DWORD dwRet )
	{
		if ( m_hWnd == NULL || pComm == NULL ) return FALSE;
		return ::PostMessage(m_hWnd, uMsg, (WPARAM)pComm, dwRet );
	}
	LRESULT SendMessage( CxSerialComm* pComm, UINT uMsg, DWORD dwRet )
	{
		if ( m_hWnd == NULL || pComm == NULL ) return FALSE;
		return ::SendMessage(m_hWnd, uMsg, (WPARAM)pComm, dwRet );
	}
	void ResetQueuedMessage( CxSerialComm* pComm )
	{
		MSG msg;
		while (::PeekMessage(&msg, m_hWnd, WM_XCOMM_CLOSE, WM_XCOMM_DISCONNECT_DEVICE, PM_REMOVE))
		{
			//TranslateMessage( &msg );
			//DispatchMessage( &msg );
		}
	}
private:
	static LRESULT CALLBACK XCOMMWGProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam );
};

CxCommWorkGroup* g_pWorkGroup = NULL;

const TCHAR CxCommWorkGroup::c_szWindowName[] = _T("A_Window_for_The_CxCommWorkGroup");

LRESULT CALLBACK CxCommWorkGroup::XCOMMWGProc( HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam )
{
	CxCommWorkGroup * pWorkGroup;
	CxSerialComm * pComm;

	switch ( iMsg )
	{
	case WM_PAINT:
		::ValidateRect(hwnd, NULL);
		break;

	case WM_XCOMM_CLOSE:
		if ( (pWorkGroup=(CxCommWorkGroup*)::GetWindowLongPtr(hwnd, GWLP_USERDATA)) == NULL ||
			(pComm=(CxSerialComm*)wParam) == NULL )
		{
			break;
		}
		pComm->OnCommClose((DWORD)lParam);
		break;

	case WM_XCOMM_RECEIVE:
		if ( (pWorkGroup=(CxCommWorkGroup*)::GetWindowLongPtr(hwnd, GWLP_USERDATA)) == NULL ||
			(pComm=(CxSerialComm*)wParam) == NULL )
		{
			break;
		}
		pComm->OnCommReceive((CxSerialComm::ReceivedPacket*)lParam);
		break;

	case WM_XCOMM_DISCONNECT_DEVICE:
		if ( (pWorkGroup=(CxCommWorkGroup*)::GetWindowLongPtr(hwnd, GWLP_USERDATA)) == NULL ||
			(pComm=(CxSerialComm*)wParam) == NULL )
		{
			break;
		}
		pComm->OnCommDisconnectDevice((DWORD)lParam);
		break;

	default:
		break;
	}

	return ::DefWindowProc(hwnd, iMsg, wParam, lParam);
}


BOOL WINAPI XCommInitialize( HINSTANCE hInstance )
{
	// register a socket window class
	if ( hInstance == NULL )
	{
		hInstance = ::GetModuleHandle(NULL);
	}
	CxCommWorkGroup::RegisterClassWnd(hInstance);

	if (!g_pWorkGroup)
		g_pWorkGroup = new CxCommWorkGroup();

	return TRUE;
}

void WINAPI XCommTerminate()
{
	if (g_pWorkGroup)
	{
		delete g_pWorkGroup;
		g_pWorkGroup = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// CxSerialComm
//////////////////////////////////////////////////////////////////////

CxSerialComm::CxSerialComm()
{
	m_pReceiveBuf = NULL;
	m_nFilledSize = 0;

	m_bCreate = FALSE;
	m_bConnected = FALSE;

	m_cPort = 1;
	m_dwBaudRate = CBR_9600;
	m_cByteSize = 8;
	m_cParity = NOPARITY;
	m_cStopBits = ONESTOPBIT;
	m_hComm = INVALID_HANDLE_VALUE;
	m_hWatchEvent = NULL;
	m_hWatchThread = NULL;
	m_osRead.hEvent = NULL;
	m_osWrite.hEvent = NULL;
	m_nThreadID = 0;

	m_nReservedPacketSize = 256*1024;		// 256kbytes (reserved)
	m_ReceivedPacket.pReceiveBuf	= new BYTE[ m_nReservedPacketSize ];
	m_ReceivedPacket.nSize			= 0;
}

CxSerialComm::~CxSerialComm()
{
	m_nReservedPacketSize = 0;
	
	if ( m_ReceivedPacket.pReceiveBuf )
	{
		delete[] m_ReceivedPacket.pReceiveBuf;
		m_ReceivedPacket.pReceiveBuf = NULL;
	}

	if (g_pWorkGroup)
		g_pWorkGroup->ResetQueuedMessage( this );
	Destroy();
}

int CxSerialComm::EnumeratePort( int* pPortArray, int nArraySize )
{
	int nPortCount = 0;

	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	BOOL bGetVer = GetVersionEx(&osvi);

	if (bGetVer && (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT))
	{
		TCHAR szDevices[65535];
		DWORD dwChars = QueryDosDevice(NULL, szDevices, 65535);
		if (dwChars)
		{
			int i=0;

			for (;;)
			{
				TCHAR* pszCurrentDevice = &szDevices[i];

				int nLen = (int)_tcslen(pszCurrentDevice);
				if (nLen > 3 && _tcsnicmp(pszCurrentDevice, _T("COM"), 3) == 0)
				{
					int nPort = _ttoi(&pszCurrentDevice[3]);
					if ( nPort != 0 )
					{
						if (pPortArray != NULL && (nArraySize > nPortCount))
							pPortArray[nPortCount] = nPort;
						++nPortCount;
					}
				}

				while (szDevices[i] != _T('\0'))
					i++;

				i++;

				if (szDevices[i] == _T('\0'))
					break;
			}
		}
		else
			XTRACE( _T("Failed in call to QueryDosDevice, GetLastError:%d\r\n"), GetLastError() );
	}
	else
	{
		for (UINT i=1; i<256; i++)
		{
			//Form the Raw device name
			CxString sPort;
			sPort.Format(_T("\\\\.\\COM%d"), i);

			BOOL bSuccess = FALSE;
			HANDLE hPort = ::CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
			if (hPort == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = GetLastError();

				if (dwError == ERROR_ACCESS_DENIED || dwError == ERROR_GEN_FAILURE)
					bSuccess = TRUE;
			}
			else
			{
				bSuccess = TRUE;
				CloseHandle(hPort);
			}

			if (bSuccess)
			{
				if (pPortArray != NULL && (nArraySize > nPortCount))
					pPortArray[nPortCount] = i;
				++nPortCount;
			}
		}
	}

	return nPortCount;
}

HANDLE CxSerialComm::GetHandle()
{
	return m_hComm;
}

BOOL CxSerialComm::IsOpen()
{
	return m_hComm != INVALID_HANDLE_VALUE;
}

BOOL CxSerialComm::IsConnected()
{
	return m_bConnected;
}

BOOL CxSerialComm::IsCreate()
{
	return m_bCreate;
}

BOOL CxSerialComm::Setup()
{
	BOOL	bRetVal;
	BOOL	bSet;
	DCB		dcb;
	
	if ( m_hComm == INVALID_HANDLE_VALUE )
		return FALSE;
	
	dcb.DCBlength = sizeof( DCB );
	
	::GetCommState( m_hComm, &dcb );
	
	dcb.BaudRate = m_dwBaudRate;
	dcb.ByteSize = m_cByteSize;
	dcb.Parity = m_cParity;
	dcb.StopBits = m_cStopBits;
	
	// setup hardware flow control
	
	bSet = (BOOL)( (m_cFlowCtrl & FC_DTRDSR) != 0 );
	dcb.fOutxDsrFlow = bSet;
	if ( bSet )
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	else
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	
	bSet = (BOOL)( (m_cFlowCtrl & FC_RTSCTS) != 0 );
	dcb.fOutxCtsFlow = bSet;
	if ( bSet )
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	else
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
	
	// setup software flow control
	bSet = (BOOL)( (m_cFlowCtrl & FC_XONXOFF) != 0 );
	
	dcb.fInX = dcb.fOutX = bSet;
	dcb.XonChar = 0x00;
	dcb.XoffChar = 0x00;
	dcb.XonLim = 100;
	dcb.XoffLim = 100;
	
	// other various settings
	dcb.fBinary = TRUE;
	dcb.fParity = FALSE;
	
	bRetVal = ::SetCommState( m_hComm, &dcb ) ;
	
	return bRetVal;
}

BOOL CxSerialComm::CreateInternal()
{
	// initialize TTY info structure
	
	m_hComm = INVALID_HANDLE_VALUE;
	m_bConnected = FALSE;
	
	m_cFlowCtrl = FC_NONE;

	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;
	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;
	
	// create I/O event used for overlapped reads / writes
	m_osRead.hEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );

	if ( m_osRead.hEvent == NULL )
	{
		return FALSE;
	}

	m_osWrite.hEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );

	if ( m_osWrite.hEvent == NULL )
	{
		::CloseHandle( m_osRead.hEvent );
		return FALSE;
	}

	m_bCreate = TRUE;
	return TRUE;
}

BOOL CxSerialComm::Create( long lPort, DWORD dwBaudRate /*= CBR_4800*/, BYTE byByteSize /*= 8*/, BYTE byStopBits /*= ONESTOPBIT*/, BYTE byParity /*= NOPARITY*/ )
{
	Destroy();

	m_cPort = (BYTE)lPort;
	m_cParity = byParity;
	m_cStopBits = byStopBits;
	m_dwBaudRate = dwBaudRate;
	m_cByteSize = byByteSize;
	BOOL dwRet = CreateInternal();
	if ( dwRet == FALSE )
	{
		return dwRet;
	}

	XASSERT( m_pReceiveBuf == NULL );

	m_pReceiveBuf = new BYTE[ MAX_BUF_SIZE ];

	if (!g_pWorkGroup)
	{
		XTRACE( _T("CxSerialComm::Create - Error \"XCommInit()\" missing\n") );
		return FALSE;
	}

	if ( !g_pWorkGroup->CreateCommWindow() )
	{
		XTRACE( _T("CxSerialComm::Create - Error \"XCommInit()\" missing\n") );
		return FALSE;
	}

	return dwRet;
}

int CxSerialComm::WriteCommBlock( BYTE* pData, DWORD dwBytesToWrite )
{
	BOOL        bWriteStat;
	DWORD       dwBytesWritten;
	DWORD       dwBytesSent = 0;
	LPOVERLAPPED pOverlap = NULL;

	if ( !IsConnected() )
		return 0;

	pOverlap = &m_osWrite;

	CheckModemStatus();	

	bWriteStat = ::WriteFile( m_hComm, pData, dwBytesToWrite, &dwBytesWritten, pOverlap );
	
	DWORD       dwErrorFlags;
	COMSTAT     ComStat;
	DWORD   	dwError;
	TCHAR       szError[128];
	
	if ( !bWriteStat )
	{
		if ( (dwError = ::GetLastError()) == ERROR_IO_PENDING )
		{
			while ( !::GetOverlappedResult( m_hComm,	&m_osWrite, &dwBytesWritten, TRUE ) )
			{
				dwError = ::GetLastError();
				if ( dwError == ERROR_IO_INCOMPLETE )
				{
					// normal result if not finished
					dwBytesSent += dwBytesWritten;
					continue;
				}
				else
				{
					CheckModemStatus();
					// an error occurred, try to recover
					wsprintf( szError, TEXT("<CE-%u>"), dwError );
					XTRACE( TEXT("%s\r\n"), szError );

					::ClearCommError( m_hComm, &dwErrorFlags, &ComStat );
					if ( dwErrorFlags > 0 )
					{
						wsprintf( szError, TEXT("<CE-%u>"), dwErrorFlags ) ;
						XTRACE( TEXT("%s\r\n"), szError );
					}
					break;
				}
			}
			
			dwBytesSent += dwBytesWritten;
			
			if ( dwBytesSent != dwBytesToWrite )
				wsprintf(szError, TEXT("Probable Write Timeout: Total of %ld bytes sent"), dwBytesSent);
			else
				wsprintf(szError, TEXT("%ld bytes written"), dwBytesSent);
			
			//XTRACE( TEXT("%s\r\n"), szError );
		}
		else
		{
			CheckModemStatus();
			// some other error occurred
			::ClearCommError( m_hComm, &dwErrorFlags, &ComStat );
			if ( dwErrorFlags > 0)
			{
				wsprintf( szError, TEXT("<CE-%u>"), dwErrorFlags );
				XTRACE( TEXT("%s: %d\r\n"), szError, dwError );
			}
			return FALSE;
		}
	}

	CheckModemStatus();

	//return bWriteStat;
	return dwBytesWritten;
}

int CxSerialComm::ReadCommBlock( BYTE* pBlock, int nMaxLength )
{
	BOOL       bReadStat;
	COMSTAT    ComStat;
	DWORD      dwErrorFlags;
	DWORD      dwLength;
	LPOVERLAPPED lpOverlap = NULL;

	lpOverlap = &m_osRead;
	
	// only try to read number of bytes in queue
	::ClearCommError( m_hComm, &dwErrorFlags, &ComStat );
	dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;
	
	if ( dwLength > 0 )
	{
		bReadStat = ::ReadFile( m_hComm, pBlock, dwLength, &dwLength, lpOverlap );

		DWORD      dwError;
		TCHAR      szError[ 10 ];
		
		if ( !bReadStat )
		{
			if ( ::GetLastError() == ERROR_IO_PENDING )
			{
				XTRACE( TEXT("IO Pending\r\n") );
				// We have to wait for read to complete.
				// This function will timeout according to the
				// CommTimeOuts.ReadTotalTimeoutConstant variable
				// Every time it times out, check for port errors
				while ( !GetOverlappedResult( m_hComm, &m_osRead, &dwLength, TRUE ) )
				{
					dwError = ::GetLastError();
					if ( dwError == ERROR_IO_INCOMPLETE )
						// normal result if not finished
						continue;
					else
					{
						// an error occurred, try to recover
						wsprintf( szError, TEXT("<CE-%u>"), dwError );
						XTRACE( TEXT("%s\r\n"), szError );
						::ClearCommError( m_hComm, &dwErrorFlags, &ComStat );
						if ( dwErrorFlags > 0 )
						{
							wsprintf( szError, TEXT("<CE-%u>"), dwErrorFlags ) ;
							XTRACE( TEXT("%s\r\n"), szError );
						}
						break;
					}
				}
			}
			else
			{
				// some other error occurred
				dwLength = 0 ;
				::ClearCommError( m_hComm, &dwErrorFlags, &ComStat );
				if ( dwErrorFlags > 0 )
				{
					wsprintf( szError, TEXT("<CE-%u>"), dwErrorFlags ) ;
					XTRACE( TEXT("%s\r\n"), szError );
				}
			}
		}
	}
	
	return dwLength;
}

void CxSerialComm::CheckModemStatus()
{
	return;
	DWORD dwEvtMask;
	// Retrieve modem control-register values.
	::GetCommModemStatus( m_hComm, &dwEvtMask );

	#ifdef _DEBUG
		CxString strEvt;
		if ( (dwEvtMask & MS_DSR_ON) != 0 )
			strEvt += TEXT("DSR ");
		if ( (dwEvtMask & MS_RLSD_ON) != 0 )
			strEvt += TEXT("RLSD ");
		if ( (dwEvtMask & MS_CTS_ON) != 0 )
			strEvt += TEXT("CTS ");
		if ( (dwEvtMask & MS_RING_ON) != 0 )
			strEvt += TEXT("RING ");

		strEvt += TEXT("\r\n");
		//XTRACE( strEvt );
	#endif

	if ( (dwEvtMask & (MS_DSR_ON|MS_RLSD_ON)) == 0 && (dwEvtMask & MS_CTS_ON) != 0 )
	{
		XTRACE( _T("ERROR: MS_CTS_ON\r\n") );
		// Disconnect Device
		DWORD dwRet = 0x00;
		if (g_pWorkGroup)
			g_pWorkGroup->PostMessage( this, WM_XCOMM_DISCONNECT_DEVICE, dwRet );
	}

	if ( (dwEvtMask & (MS_RING_ON|MS_RLSD_ON)) == (MS_RING_ON|MS_RLSD_ON) )		// cable disconnect
	{
		XTRACE( _T("ERROR: MS_RING_ON|MS_RLSD_ON\r\n") );
		// Disconnect Device
		DWORD dwRet = 0x00;
		if (g_pWorkGroup)
			g_pWorkGroup->PostMessage( this, WM_XCOMM_DISCONNECT_DEVICE, dwRet );
	}
}

unsigned int __stdcall CxSerialComm::CommWatchProc( LPVOID lpData )
{
	DWORD		dwEvtMask;
	int			nLength;
	BYTE		pcIn[ MAXBLOCK + 1];
	CxSerialComm*		pComm = (CxSerialComm*) lpData;

	DWORD		dwSetEvtmask;

	OVERLAPPED	os;	
	memset( &os, 0, sizeof( OVERLAPPED ) );
	
	// create I/O event used for overlapped read
	os.hEvent = ::CreateEvent( NULL, TRUE, FALSE, NULL );
	if (os.hEvent == NULL)
	{
		XTRACE( TEXT("Failed to create event for thread!\r\n") );
		return FALSE;
	}

	dwSetEvtmask = EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING;
	
	if ( !::SetCommMask( pComm->m_hComm, dwSetEvtmask ) )
		return FALSE;

	BOOL bCheckOK = FALSE;

	while ( pComm->IsConnected() )
	{
		dwEvtMask = 0;

		pComm->CheckModemStatus();

		::SetCommMask( pComm->m_hComm, dwSetEvtmask );

		BOOL bWaitOK = ::WaitCommEvent( pComm->m_hComm, &dwEvtMask, NULL );
		if ( bWaitOK )
		{
			//XTRACE( _T("dwEvtMask: %X\r\n"), dwEvtMask );
			if ( (dwEvtMask & EV_RXCHAR) == EV_RXCHAR )
			{
				do
				{
					if ( nLength = pComm->ReadCommBlock( pcIn, MAXBLOCK ) )
					{
						pComm->OnReceive( pcIn, nLength ) ;
					}
				}
				while ( nLength > 0 );
			}
		}
		else
		{
			XTRACE( _T("ERROR: WaitCommEvent\r\n") );
			if (g_pWorkGroup)
				g_pWorkGroup->PostMessage( pComm, WM_XCOMM_DISCONNECT_DEVICE, ::GetLastError() );
			break;
		}

	}
	
	// get rid of event handle
	::CloseHandle( os.hEvent ) ;
	
	// clear information in structure (kind of a "we're done flag")
	pComm->m_nThreadID = 0;
	pComm->m_hWatchThread = NULL;

	// notify close
	DWORD dwRet = 0x00;
	if (g_pWorkGroup)
		g_pWorkGroup->PostMessage( pComm, WM_XCOMM_CLOSE, dwRet );

	::_endthreadex(0);
	
	return TRUE;
}

BOOL CxSerialComm::Open()
{
	if ( IsOpen() )
		Close();
	
	TCHAR szComFile[16];

	if ( m_cPort <= 9 )
		wsprintf( szComFile, _T("COM%d:"), m_cPort );
	else
		wsprintf( szComFile, _T("\\\\.\\COM%d"), m_cPort );
	
	DWORD dwFlag = 0;

	dwFlag = /*FILE_ATTRIBUTE_NORMAL | */FILE_FLAG_OVERLAPPED;
	
	if ( ( m_hComm  =
		::CreateFile( szComFile, GENERIC_READ | GENERIC_WRITE,
		0,                    // exclusive access
		NULL,                 // no security attrs
		OPEN_EXISTING,
		dwFlag,
		NULL ) ) == INVALID_HANDLE_VALUE )
		return FALSE;
	else
	{
		// get any early notifications
		::SetCommMask( m_hComm, EV_RXCHAR | EV_CTS | EV_DSR | EV_RLSD | EV_RING );
		
		// setup device buffers
		::SetupComm( m_hComm, 4096, 4096 );
		
		// purge any information in the buffer
		::PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );
		
		COMMTIMEOUTS  CommTimeOuts;
		// set up for overlapped I/O
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = 0;
		// CBR_9600 is approximately 1byte/ms. For our purposes, allow
		// double the expected time per character for a fudge factor.
		CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/m_dwBaudRate;
		CommTimeOuts.WriteTotalTimeoutConstant = 0;
		::SetCommTimeouts( m_hComm, &CommTimeOuts );
	}
	
	HANDLE	hCommWatchThread = NULL;
	unsigned int nThreadID;
	
	if ( Setup() )
	{
		m_bConnected = TRUE;
		
		// Create a secondary thread
		// to watch for an event.
		if ( NULL == (hCommWatchThread =
				(HANDLE)::_beginthreadex( (LPSECURITY_ATTRIBUTES) NULL,
								0,
								CommWatchProc,
								(LPVOID) this,
								0, &nThreadID ) ) )
		{
			m_bConnected = FALSE;
			::CloseHandle( m_hComm ) ;
			return FALSE;
		}
		else
		{
			m_nThreadID = nThreadID;
			m_hWatchThread = hCommWatchThread;
		
			// assert DTR
			::EscapeCommFunction( m_hComm, SETDTR );
		}
	}
	else
	{
		m_bConnected = FALSE;
		::CloseHandle( m_hComm );
	}
	
	return TRUE;
}

BOOL CxSerialComm::Close()
{
	if ( !IsConnected() )
		return FALSE;

	// disable event notification and wait for thread
	// to halt
	::SetCommMask( m_hComm, 0 );
	
	m_bConnected = FALSE;
	// block until thread has been halted
	if ( m_hWatchThread )
	{
		if ( ::WaitForSingleObject( m_hWatchThread, 1000 ) != WAIT_OBJECT_0 )
		{
			XTRACE( _T("CxSerialComm::Close() - TerminateThread\r\n") );
			TerminateThread( m_hWatchThread, 0 );
		}

		::CloseHandle( m_hWatchThread );
		m_hWatchThread = NULL;
	}

	//while ( m_hWatchThread != NULL ) ;
	
	// drop DTR
	::EscapeCommFunction( m_hComm, CLRDTR );
	
	// purge any outstanding reads/writes and close device handle
	::PurgeComm( m_hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

	::CloseHandle( m_hComm );

	m_hComm = INVALID_HANDLE_VALUE;
	
	return TRUE;
}

BOOL CxSerialComm::Destroy()
{
	if ( !IsCreate() )
		return FALSE;

   if ( IsOpen() )
	   Close();

   // clean up event objects
   CloseHandle( m_osRead.hEvent );
   CloseHandle( m_osWrite.hEvent );

   if ( m_pReceiveBuf != NULL )
   {
	   delete[] m_pReceiveBuf;
	   m_pReceiveBuf = NULL;
   }

   m_nFilledSize = 0;

   m_bCreate = FALSE;

   return TRUE;
}


void CxSerialComm::OnReceive( BYTE* pData, int nSize ) 
{ 
//	XTRACE( TEXT("Received: %d\r\n"), nSize );

	if ( m_nFilledSize + nSize >= MAX_BUF_SIZE )
	{
		m_nFilledSize = 0;
	}

	memcpy( m_pReceiveBuf+m_nFilledSize, pData, nSize );
	m_nFilledSize += nSize;

	ParsePacket( m_pReceiveBuf, m_nFilledSize );
}

CxSerialComm::ReceivedPacket* CxSerialComm::GetPacket( int nSTXPos, int nETXPos )
{
	m_ReceivedPacket.nSize = nETXPos-nSTXPos+1;
	if ( m_ReceivedPacket.nSize <= 0 ) return NULL;
	
	if ( m_nReservedPacketSize < m_ReceivedPacket.nSize )
	{
		delete[] m_ReceivedPacket.pReceiveBuf;
		m_ReceivedPacket.pReceiveBuf = new BYTE[m_ReceivedPacket.nSize];
	}

	memcpy( m_ReceivedPacket.pReceiveBuf, m_pReceiveBuf+nSTXPos, m_ReceivedPacket.nSize );

	int nRemindSize = m_nFilledSize-nETXPos-1;

	if ( nRemindSize <= 0 )
		m_nFilledSize = 0;
	else
	{
		memcpy( m_pReceiveBuf, m_pReceiveBuf+nETXPos+1, nRemindSize );
		m_nFilledSize = nRemindSize;
	}

	return &m_ReceivedPacket;
}

void CxSerialComm::ParsePacket( BYTE* pReceiveBuffer, int nBufferSize )
{
	if ( nBufferSize <= 0 ) return;
	int nSTXPos = OnFindSTX( pReceiveBuffer, nBufferSize );
	if ( nSTXPos != NOT_DEFINE_SYMBOL )
	{
		if ( nSTXPos < 0 ) return;		// shortage
	}
	else
	{
		nSTXPos = 0;
	}

	int nPacketSize = OnGetPacketSize( pReceiveBuffer+nSTXPos, nBufferSize-nSTXPos );

	if ( nPacketSize == NOT_DEFINE_SYMBOL )
	{
		int nETXPos = OnFindETX( pReceiveBuffer, nBufferSize );
		if ( nETXPos != NOT_DEFINE_SYMBOL )
		{
			if ( nETXPos < 0 ) return;
		}
		else
		{
			nETXPos = nBufferSize-1;
		}

		ReceivedPacket* pPacket = GetPacket( nSTXPos, nETXPos );
		if ( pPacket )
		{
			if (g_pWorkGroup)
				g_pWorkGroup->PostMessage( this, WM_XCOMM_RECEIVE, (DWORD)pPacket );
			ParsePacket( m_pReceiveBuf, m_nFilledSize );
		}
		return;
	}
	else
	{
		if ( nPacketSize <= 0 ) return;		// shortage

		if ( nSTXPos+nPacketSize > m_nFilledSize )
			return;		// shortage

		int nETXPos = nSTXPos+nPacketSize-1;
		ReceivedPacket* pPacket = GetPacket( nSTXPos, nETXPos );
		if ( pPacket )
		{
			if (g_pWorkGroup)
				g_pWorkGroup->PostMessage( this, WM_XCOMM_RECEIVE, (DWORD)pPacket );
			ParsePacket( m_pReceiveBuf, m_nFilledSize );
		}
		return;
	}

	int nETXPos = OnFindETX( pReceiveBuffer, nBufferSize );

	if ( nETXPos == NOT_DEFINE_SYMBOL )
	{
		nETXPos = m_nFilledSize-1;
		ReceivedPacket* pPacket = GetPacket( nSTXPos, nETXPos );
		if ( pPacket )
		{
			if (g_pWorkGroup)
				g_pWorkGroup->PostMessage( this, WM_XCOMM_RECEIVE, (DWORD)pPacket );
			ParsePacket( m_pReceiveBuf, m_nFilledSize );
		}
		return;
	}
	else
	{
		if ( nETXPos < 0 ) return;		// shortage
		else
		{
			ReceivedPacket* pPacket = GetPacket( nSTXPos, nETXPos );
			if ( pPacket )
			{
				if (g_pWorkGroup)
					g_pWorkGroup->PostMessage( this, WM_XCOMM_RECEIVE, (DWORD)pPacket );
				ParsePacket( m_pReceiveBuf, m_nFilledSize );
			}
			return;
		}
	}
}