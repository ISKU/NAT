#pragma once
#include "baselayer.h"

class CTCPLayer : public CBaseLayer
{
public:
	CTCPLayer(char* pName = NULL);
	virtual ~CTCPLayer();

	void SetPseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length);
	unsigned short SetChecksum(int nlength);
	BOOL IsValidChecksum(unsigned char* p_header, unsigned short checksum, int nlength);

	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	typedef struct _TCP {
		unsigned short Tcp_srcPort;
		unsigned short Tcp_dstPort;
		unsigned int Tcp_sequenceNumber;
		unsigned int Tcp_ackNumber;
		unsigned char Tcp_lengthAndReserved;
		unsigned char Tcp_flags;
		unsigned short Tcp_windowSize;
		unsigned short Tcp_checksum;
		unsigned short Tcp_urgentPointer;
		unsigned char Tcp_data[TCP_MAX_DATA];
	} TcpHeader, *PTcpHeader;

	typedef struct _TCP_Pseudo_Header {
		unsigned char Pseudo_srcIp[4];
		unsigned char Pseudo_dstIp[4];
		unsigned char Pseudo_unused;
		unsigned char Pseudo_protoId;
		unsigned short Pseudo_length;
	} TcpPseudoHeader, *PTcpPseudoHeader;

	TcpPseudoHeader Tcp_pseudo_header;


private:
	void ResetPseudoHeader();

	PTcpHeader receivedPacket;
};