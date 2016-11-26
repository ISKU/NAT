#pragma once
#include "baselayer.h"

class CIPLayer : public CBaseLayer  
{
public:
	CIPLayer(char* pName = NULL);
	virtual ~CIPLayer();

	unsigned char* GetDstIP(int dev_num);
	void SetSrcPacketIP(unsigned char* ip);
	void SetDstPacketIP(unsigned char* ip);
	unsigned char* GetSrcFromPacket();
	unsigned char* GetDstFromPacket();
	unsigned short SetChecksum(unsigned char p_header[20]);
	BOOL IsValidChecksum(unsigned char* received_header, unsigned short checksum);
	void SetDstIP(unsigned char* ip, int dev_num);

	unsigned short GetLength();

	BOOL Send(unsigned char* ppayload, int nlength,int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	// IP 헤더를 나타내는 구조체
	typedef struct _IP {
		unsigned char Ip_version;
		unsigned char Ip_typeOfService;
		unsigned short Ip_len;
		unsigned short Ip_id; // TCP_DATA_SIZE = 1560 bytes
		unsigned short Ip_fragmentOffset;
		unsigned char Ip_timeToLive;
		unsigned char Ip_protocol;
		unsigned short Ip_checksum;
		union {
			unsigned char Ip_srcAddressByte[4];
			unsigned int Ip_srcAddress;
		};
		union {
			unsigned char Ip_dstAddressByte[4];
			unsigned int Ip_dstAddress;
		};
		unsigned char Ip_data[IP_MAX_DATA];
	} IpHeader, *PIpHeader;

private:
	PIpHeader receivedPacket;
};
