#pragma once
#include "baselayer.h"

class CRIPLayer : public CBaseLayer
{
public:
	CRIPLayer(char* pName = NULL);
	virtual ~CRIPLayer();

	BOOL Send(int command,int dev_num, int resend);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	typedef struct _RIPTable {
		unsigned short Rip_family;
		unsigned short Rip_tag;
		unsigned char Rip_ipAddress[4];
		unsigned char Rip_subnetmask[4];
		unsigned char Rip_nexthop[4];
		unsigned int Rip_metric;
	} RipTable, *PRipTable;

	typedef struct _RIP {
		unsigned char Rip_command;
		unsigned char Rip_version;
		unsigned short Rip_reserved;
		RipTable Rip_table[25];
	} RipHeader, *PRipHeader;

private:
	unsigned char dev_1_udp_addr[4];
	unsigned char dev_2_udp_addr[4];
	inline void	ResetHeader();
	inline void CreateRequestMessage();
	inline int CreateResponseMessageTable(int dev_num);

	int ContainsRouteTableEntry(unsigned char Ip_addr[4]);
	RipHeader Rip_header;
};