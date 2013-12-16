// MCSocket.h: interface for the CxMCSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MCSOCKET_H__02385D28_A7C8_4854_B2B4_833015465D97__INCLUDED_)
#define AFX_MCSOCKET_H__02385D28_A7C8_4854_B2B4_833015465D97__INCLUDED_

#include <XUtil/Comm/xClientSocketTCP.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma pack(push, 1)
struct stQHeader
{
	BYTE	NetNumber;
	BYTE	PLCNumber;
	WORD	IONumber;
	BYTE	StationNumber;
	WORD	Length;		// little-endian
};

struct stPLCSendHeader
{
	BYTE		SubHeader[2];
	stQHeader	QHeader;
	BYTE		Reserved[2];
	BYTE		Command[2];
	BYTE		SubCommand[2];
	BYTE		Data[1];
};

struct stPLCResponsHeader
{
	BYTE		SubHeader[2];
	stQHeader	QHeader;
	WORD		ExitState;
	BYTE		Data[1];
};

#pragma pack(pop, 1)

#define		MAX_WORD_COUNT			(100)

class XUTIL_API CxMCSocket : public CxClientSocketTCP
{
private:
	BYTE	m_cNetNumber;
	BYTE	m_cPLCNumber;
	WORD	m_wIONumber;
	BYTE	m_cStationNumber;

	HANDLE	m_hReceiveEvent;

	WORD*	m_pReadBuffer;
	UINT	m_nToReadCount;

	BOOL	m_bWriteOK;
protected:
	virtual void ProcessBuffer( ClientThreadContext* pContext, char* pReceiveBuf, int nRecvBytes );
public:
	CxMCSocket();
	virtual ~CxMCSocket();

	void SetConfig( BYTE cNetNumber, BYTE cPLCNumber, WORD wIONumber, BYTE cStationNumber );

	BOOL WriteBuffer( DWORD dwAddress, UINT nCount, WORD* pBuffer, UINT nTimeout );	// sync
	BOOL WriteBit( DWORD dwAddress, int nBit, BYTE cValue, UINT nTimeout );
	BOOL ReadBuffer( DWORD dwAddress, UINT nCount, WORD* pBuffer, UINT nTimeout );	// sync
	virtual void OnPacketReceived( ClientThreadContext* pContext, CxIoBuffer* PacketBuffer );
};

#endif // !defined(AFX_MCSOCKET_H__02385D28_A7C8_4854_B2B4_833015465D97__INCLUDED_)
