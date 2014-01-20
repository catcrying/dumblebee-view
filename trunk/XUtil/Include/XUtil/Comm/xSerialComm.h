// xSerialComm.h: interface for the CxSerialComm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NCOMM_H__DE259074_44C5_416B_B140_48127C87B32E__INCLUDED_)
#define AFX_NCOMM_H__DE259074_44C5_416B_B140_48127C87B32E__INCLUDED_

//////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAXBLOCK        80

#define FC_NONE			0x00
#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04

#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/export.h>

#define	NOT_DEFINE_SYMBOL	(-0x4c0de)

XUTIL_API BOOL WINAPI XCommInit( HINSTANCE hInstance = NULL );

class XUTIL_API CxSerialComm
{
public:
	typedef struct _ReceivedPacket
	{
		int		nSize;
		BYTE*	pReceiveBuf;
	} ReceivedPacket;
	
protected:
	BYTE*			m_pReceiveBuf;
	int				m_nFilledSize;
	ReceivedPacket	m_ReceivedPacket;
	int				m_nReservedPacketSize;

	HANDLE		m_hComm;
	BYTE		m_cPort;
	volatile BOOL		m_bConnected;
	BYTE		m_cByteSize, m_cFlowCtrl, m_cParity, m_cStopBits;
	DWORD		m_dwBaudRate;
	HANDLE		m_hWatchEvent;
	HANDLE		m_hWatchThread;
	unsigned int	m_nThreadID;

	OVERLAPPED	m_osWrite, m_osRead;

	BOOL		m_bCreate;

	BOOL Setup();
	BOOL CreateInternal();

	LARGE_INTEGER m_ltPacketIdleTime;

	virtual void ParsePacket( BYTE* pReceiveBuffer, int nBufferSize );
	ReceivedPacket* GetPacket( int nSTXPos, int nETXPos );

public:
	CxSerialComm();
	virtual ~CxSerialComm();

	static int EnumeratePort( int* pPortArray, int nArraySize );

	void CheckModemStatus();

	BOOL Destroy();
	BOOL Create( long lPort, DWORD dwBaudRate = CBR_4800, BYTE byByteSize = 8, BYTE byStopBits = ONESTOPBIT, BYTE byParity = NOPARITY );

	int ReadCommBlock( BYTE* pBlock, int nMaxLength );
	int WriteCommBlock( BYTE* pData, DWORD dwBytesToWrite );

	BOOL Open();
	BOOL Close();

	HANDLE GetHandle();
	BOOL IsOpen();
	BOOL IsConnected();

	BOOL IsCreate();

	virtual void OnReceive( BYTE* pData, int nSize );

	virtual int OnFindSTX( BYTE* pReceiveBuffer, int nBufferSize ) { return NOT_DEFINE_SYMBOL; }
	virtual int OnFindETX( BYTE* pReceiveBuffer, int nBufferSize ) { return NOT_DEFINE_SYMBOL; }
	virtual int OnGetPacketSize( BYTE* pReceiveBuffer, int nBufferSize ) { return NOT_DEFINE_SYMBOL; }

	virtual void OnCommClose ( DWORD dwRet ) = 0;
	virtual void OnCommReceive ( ReceivedPacket* pPacket ) = 0;
	virtual void OnCommDisconnectDevice( DWORD dwRet ) = 0;

	static unsigned int __stdcall CommWatchProc( LPVOID lpData );
};


#endif // !defined(AFX_NCOMM_H__DE259074_44C5_416B_B140_48127C87B32E__INCLUDED_)
