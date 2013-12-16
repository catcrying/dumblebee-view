// MCSocket.cpp: implementation of the CxMCSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <XUtil/Comm/xMCSocket.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CxMCSocket::CxMCSocket()
{
	m_cNetNumber		= 0;
	m_cPLCNumber		= 0;
	m_wIONumber			= 0;
	m_cStationNumber	= 0;

	m_hReceiveEvent		= ::CreateEvent( NULL, TRUE, FALSE, NULL );

	m_pReadBuffer		= new WORD[MAX_WORD_COUNT];
	m_nToReadCount		= 0;
}

CxMCSocket::~CxMCSocket()
{
	if (m_hReceiveEvent != NULL)
	{
		::CloseHandle( m_hReceiveEvent );
		m_hReceiveEvent = NULL;
	}

	if (m_pReadBuffer)
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}
}

void CxMCSocket::SetConfig( BYTE cNetNumber, BYTE cPLCNumber, WORD wIONumber, BYTE cStationNumber )
{
	m_cNetNumber		= cNetNumber;
	m_cPLCNumber		= cPLCNumber;
	m_wIONumber			= wIONumber;
	m_cStationNumber	= cStationNumber;
}

void CxMCSocket::ProcessBuffer( ClientThreadContext* pContext, char* pReceiveBuf, int nRecvBytes )
{
	if ( !pContext->IoBuffer.Add( pReceiveBuf, nRecvBytes ) )
	{
		// ERROR
		XTRACE( _T("ReceiveBuffer Overflow!\r\n") );
		pContext->IoBuffer.Clear();
	}
	
	int nSize;
	
	while ( (nSize=pContext->IoBuffer.GetSize()) >= sizeof(stQHeader)+2 )
	{
		stPLCResponsHeader* pHeader = (stPLCResponsHeader*)pContext->IoBuffer.GetBuffer();
		
		int nPacketSize = pHeader->QHeader.Length + sizeof(stQHeader) + 2;
		
		if ( nPacketSize <= nSize )
		{
			// Complete Packet
			CxIoBuffer PacketBuffer;
			PacketBuffer.Create();
			PacketBuffer.Add( const_cast<char*>(pContext->IoBuffer.GetBuffer()), nPacketSize );
			pContext->IoBuffer.Remove( 0, nPacketSize-1 );
			OnPacketReceived( pContext, &PacketBuffer );
		}
		else // nPacketSize > nSize
		{
			return;
		}
	}
}

void CxMCSocket::OnPacketReceived( ClientThreadContext* pContext, CxIoBuffer* PacketBuffer ) 
{
	XTRACE( _T("Received\r\n") );
	stPLCResponsHeader* pHeader = (stPLCResponsHeader*)PacketBuffer->GetBuffer();

	if ( pHeader->ExitState == 0 )
	{
		if ( pHeader->QHeader.Length > 2 )
		{
			
			UINT nToReadCount = (pHeader->QHeader.Length - 2) / 2;
			//memcpy( m_pReadBuffer, pHeader->Data, (pHeader->QHeader.Length - 2) );
			XTRACE( _T("m_nToReadCount = %d, nToReadCount = %d\r\n"), m_nToReadCount, nToReadCount );
			if ( m_nToReadCount == nToReadCount )
				memcpy( m_pReadBuffer, pHeader->Data, (pHeader->QHeader.Length - 2) );
		}
		else
		{
			m_bWriteOK = TRUE;
		}
	}
	else
	{
		if ( m_nToReadCount != 0 )
		{
			memset( m_pReadBuffer, 0, sizeof(WORD)*MAX_WORD_COUNT );
		}
		else
		{
			m_bWriteOK = FALSE;
		}
	}

	::SetEvent( m_hReceiveEvent );
}

BOOL CxMCSocket::WriteBuffer( DWORD dwAddress, UINT nCount, WORD* pBuffer, UINT nTimeout )
{
	m_nToReadCount = 0;
	m_bWriteOK = TRUE;
	int nPacketSize = 2 + sizeof(stQHeader) + 6 + 4 + 2 + nCount*2;

	stPLCSendHeader* pHeader = (stPLCSendHeader*)malloc( nPacketSize );
	
	pHeader->SubHeader[0] = 0x50;
	pHeader->SubHeader[1] = 0x00;
	
	pHeader->QHeader.NetNumber = m_cNetNumber;
	pHeader->QHeader.PLCNumber = m_cPLCNumber;
	pHeader->QHeader.IONumber = m_wIONumber;
	pHeader->QHeader.StationNumber = m_cStationNumber;
	pHeader->QHeader.Length = nPacketSize - 2 - (WORD)sizeof(stQHeader);
	pHeader->Reserved[0] = 0x10;	pHeader->Reserved[1] = 0x00;	// CPU Timer, if 0 = infinite.
	pHeader->Command[0] = 0x13;		pHeader->Command[1] = 0x16;		// Low Byte first.
	pHeader->SubCommand[0] = 0x00;	pHeader->SubCommand[1] = 0x00;
	

	pHeader->Data[0] = BYTE(dwAddress & 0xFF);			pHeader->Data[1] = BYTE((dwAddress >> 8) & 0xFF);
	pHeader->Data[2] = BYTE((dwAddress >> 16) & 0xFF);	pHeader->Data[3] = BYTE((dwAddress >> 24) & 0xFF);
	
	pHeader->Data[4] = nCount & 0xFF;	pHeader->Data[5] = (nCount >> 8) & 0xFF;
	
	memcpy( pHeader->Data+6, pBuffer, sizeof(WORD)*nCount );
	
	::ResetEvent( m_hReceiveEvent );
	SendBuffer( &m_ThreadContext, (char*)pHeader, nPacketSize );
	free( pHeader );

	int nTCnt = nTimeout/10;
	MSG msg;
	for ( int i=0 ; i<nTCnt ; i++ )
	{
		if ( ::WaitForSingleObject( m_hReceiveEvent, 10 ) == WAIT_OBJECT_0 )
		{
			return m_bWriteOK;
		}

		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );

		}
	}

	return FALSE;
}

BOOL CxMCSocket::WriteBit( DWORD dwAddress, int nBit, BYTE cValue, UINT nTimeout )
{
	XASSERT( cValue == 0 || cValue == 1 );
	
	WORD dwRead;
	if ( !ReadBuffer( dwAddress, 1, &dwRead, nTimeout ) ) return FALSE;
	
	dwRead &= ~(0x01<<nBit);
	dwRead |= (cValue<<nBit);
	if ( !WriteBuffer( dwAddress, 1, &dwRead, nTimeout) ) return FALSE;

	return TRUE;
}

BOOL CxMCSocket::ReadBuffer( DWORD dwAddress, UINT nCount, WORD* pBuffer, UINT nTimeout )
{
	m_nToReadCount = nCount;

	int nPacketSize = 2 + sizeof(stQHeader) + 6 + 4 + 2;
	
	stPLCSendHeader* pHeader = (stPLCSendHeader*)malloc( nPacketSize );
	
	pHeader->SubHeader[0] = 0x50;
	pHeader->SubHeader[1] = 0x00;
	
	pHeader->QHeader.NetNumber = m_cNetNumber;
	pHeader->QHeader.PLCNumber = m_cPLCNumber;
	pHeader->QHeader.IONumber = m_wIONumber;
	pHeader->QHeader.StationNumber = m_cStationNumber;
	pHeader->QHeader.Length = nPacketSize - 2 - (WORD)sizeof(stQHeader);
	pHeader->Reserved[0] = pHeader->Reserved[1] = 0x00;
	pHeader->Command[0] = 0x13;		pHeader->Command[1] = 0x06;
	pHeader->SubCommand[0] = 0x00;	pHeader->SubCommand[1] = 0x00;
	
	::ResetEvent( m_hReceiveEvent );

	pHeader->Data[0] = BYTE(dwAddress & 0xFF);			pHeader->Data[1] = BYTE((dwAddress >> 8) & 0xFF);
	pHeader->Data[2] = BYTE((dwAddress >> 16) & 0xFF);	pHeader->Data[3] = BYTE((dwAddress >> 24) & 0xFF);
	
	pHeader->Data[4] = nCount & 0xFF;	pHeader->Data[5] = (nCount >> 8) & 0xFF;
		
	Sleep(5);
	SendBuffer( &m_ThreadContext, (char*)pHeader, nPacketSize );
	free( pHeader );

	int nTCnt = nTimeout/10;
	MSG msg;
	for ( int i=0 ; i<nTCnt ; i++ )
	{
		if ( ::WaitForSingleObject( m_hReceiveEvent, 10 ) == WAIT_OBJECT_0 )
		{
			memcpy( pBuffer, m_pReadBuffer, sizeof(WORD)*m_nToReadCount );
			return TRUE;
		}

		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );

		}
	}

	return FALSE;
}

