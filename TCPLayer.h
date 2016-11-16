#pragma once
#include "baselayer.h"

class CTCPLayer : public CBaseLayer
{
public:
	CTCPLayer(char* pName = NULL);
	virtual ~CTCPLayer();

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

	TcpHeader Tcp_header;
	TcpPseudoHeader Tcp_pseudo_header;

private:
	inline void		ResetHeader();
	void			ResetPseudoHeader();

private:
	unsigned short dev_1_length;
	unsigned short dev_2_length;
	unsigned short dev_1_length_for_rip;
	unsigned short dev_2_length_for_rip;
};