#pragma once
#include "baselayer.h"
#include "afxmt.h"

class CICMPLayer : public CBaseLayer
{
public:
	CICMPLayer(char* pName = NULL);
	virtual ~CICMPLayer();

	typedef struct _ICMP {
		unsigned char Icmp_type;
		unsigned char Icmp_code;
		unsigned short Icmp_checksum;
		unsigned short Icmp_identifier;
		unsigned short Icmp_sequenceNumber;
		unsigned char Icmp_data[ICMP_MAX_DATA];
	} IcmpHeader, *PIcmpHeader;

	typedef struct _ICMP_ENTRY {
		unsigned char inner_addr[4];
		unsigned char outer_addr[4];
		unsigned short identifier;
		unsigned short sequenceNumber;
		unsigned int time;
	} ICMP_ENTRY, *PICMP_ENTRY;

	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);
	void UpdateTable();
	static unsigned int IcmpTableCheck(LPVOID pParam);

	static CList<ICMP_ENTRY,ICMP_ENTRY&> Icmp_table;
	IcmpHeader Icmp_header;

	CWinThread* pThread_1;
	void StartReadThread();
private:
	int				searchTable(unsigned short identifier, unsigned short sequenceNumber);
};