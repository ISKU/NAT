#include "StdAfx.h"
#include "TCPLayer.h"
#include "RouterDlg.h"

CTCPLayer::CTCPLayer(char* pName) : CBaseLayer(pName)
{
	ResetHeader();
	ResetPseudoHeader();
}

CTCPLayer::~CTCPLayer()
{
}

unsigned short CTCPLayer::GetSrcPort()
{
	return Tcp_header.Tcp_srcPort;
}

unsigned short CTCPLayer::GetDstPort()
{
	return Tcp_header.Tcp_dstPort;
}

void CTCPLayer::SetSrcPort(unsigned short port)
{
	Tcp_header.Tcp_srcPort = port;
}

void CTCPLayer::SetDstPort(unsigned short port)
{
	Tcp_header.Tcp_dstPort = port;
}

BOOL CTCPLayer::Send(unsigned char* ppayload, int nlength, int dev_num)
{

}

BOOL CTCPLayer::Receive(unsigned char* ppayload, int dev_num)
{

}

void CTCPLayer::ResetHeader()
{
	Tcp_header.Tcp_srcPort = 0x0000;
	Tcp_header.Tcp_dstPort = 0x0000;
	Tcp_header.Tcp_sequenceNumber = 0x00000000;
	Tcp_header.Tcp_ackNumber = 0x00000000;
	Tcp_header.Tcp_lengthAndReserved = 0x00;
	Tcp_header.Tcp_flags = 0x00;
	Tcp_header.Tcp_windowSize = 0x0000;
	Tcp_header.Tcp_checksum = 0x0000;
	Tcp_header.Tcp_urgentPointer = 0x0000;
}

void CTCPLayer::ResetPseudoHeader()
{
	Tcp_pseudo_header.Pseudo_unused = 0;
	Tcp_pseudo_header.Pseudo_protoId = 0;
}