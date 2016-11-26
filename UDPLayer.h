#pragma once
#include "baselayer.h"

class CUDPLayer : public CBaseLayer
{
public:
	CUDPLayer(char* pName = NULL);
	virtual ~CUDPLayer();

	unsigned short	GetDstPort();
	unsigned short	GetSrcPort();
	unsigned short	GetLength(int dev_num);
	unsigned short	GetLengthForRIP(int dev_num);
	void SetDstPort(unsigned short port);
	void SetSrcPort(unsigned short port);
	void SetLength(unsigned short length, int dev_num);
	void SetLengthForRIP(unsigned short length, int dev_num);
	void SetSendPseudoHeader(unsigned short length, int dev_num);
	void SetReceivePseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length);
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
	inline void		ResetHeader();
	void			ResetPseudoHeader();
	int IncomingSearchTable(unsigned char inner_addr[4], unsigned short inner_port);
	int OutgoingSearchTable(unsigned short outer_port);
	unsigned short dev_1_length;
	unsigned short dev_2_length;
	unsigned short dev_1_length_for_rip;
	unsigned short dev_2_length_for_rip;
};