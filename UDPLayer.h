#pragma once
#include "baselayer.h"

class CUDPLayer : public CBaseLayer
{
public:
	CUDPLayer(char* pName = NULL);
	virtual ~CUDPLayer();

	void SetPseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length);
	unsigned short SetChecksum(int nlength);
	BOOL IsValidChecksum(unsigned char* p_header, unsigned short checksum, int nlength);

	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	typedef struct _UDP {
		unsigned short Udp_srcPort;
		unsigned short Udp_dstPort;
		unsigned short Udp_length;
		unsigned short Udp_checksum;
		unsigned char Udp_data[UDP_MAX_DATA];
	} UdpHeader, *PUdpHeader;

	typedef struct _UDP_Pseudo_Header {
		unsigned char Pseudo_srcIp[4];
		unsigned char Pseudo_dstIp[4];
		unsigned char Pseudo_unused;
		unsigned char Pseudo_protoId;
		unsigned short Pseudo_length;
	} UdpPseudoHeader, *PUdpPseudoHeader;

	UdpPseudoHeader Udp_pseudo_header;

private:
	void			ResetPseudoHeader();
	int SearchIncomingTable(unsigned short outer_port);
	int SearchOutgoingTable(unsigned char inner_addr[4], unsigned short inner_port);

	PUdpHeader receivedPacket;
	int circularIndex;
};