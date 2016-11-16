#pragma once
#include "baselayer.h"

class CIPLayer : public CBaseLayer  
{
public:
	CIPLayer(char* pName = NULL);
	virtual ~CIPLayer();

	unsigned char* GetDstIP(int dev_num);
	unsigned char* GetSrcIP(int dev_num);
	unsigned char GetProtocol(int dev_num);
	unsigned char* GetSrcIPForRIPLayer(int dev_num);
	unsigned short SetChecksum(unsigned char p_header[20]);
	BOOL IsValidChecksum(unsigned char* received_header, unsigned short checksum);
	void SetDstIP(unsigned char* ip, int dev_num);
	void SetSrcIP(unsigned char* ip, int dev_num);
	void SetProtocol(unsigned char protocol, int dev_num);
	void SetSrcIPForRIPLayer(unsigned char* ip, int dev_num);

	BOOL Send(unsigned char* ppayload, int nlength,int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	// IP ����� ��Ÿ���� ����ü
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
	unsigned char dev_1_ip_addr[4];
	unsigned char dev_2_ip_addr[4];
	unsigned char dev_1_dst_ip_addr[4];
	unsigned char dev_2_dst_ip_addr[4];
	unsigned char dev_1_protocol;
	unsigned char dev_2_protocol;
	unsigned char dev_1_ip_addr_for_rip[4];
	unsigned char dev_2_ip_addr_for_rip[4];
	inline void	ResetHeader(); // IP ��� �ʱ�ȭ �Լ�
	inline int Forwarding(unsigned char destip[4]);
	int CIPLayer::ContainsRouteTableEntry(unsigned char Ip_addr[4]);
	IpHeader Ip_header; // IP1 ��� ����ü ����
};
