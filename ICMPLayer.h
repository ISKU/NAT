#pragma once
#include "baselayer.h"

class CICMPLayer : public CBaseLayer
{
public:
	CICMPLayer(char* pName = NULL);
	virtual ~CICMPLayer();

	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);

public:
	typedef struct _ICMP {
		unsigned char Icmp_type;
		unsigned char Icmp_code;
		unsigned short Icmp_checksum;
		unsigned short Icmp_identifier;
		unsigned short Icmp_sequenceNumber;
		unsigned char Icmp_data[ICMP_MAX_DATA];
	} IcmpHeader, *PIcmpHeader;

	IcmpHeader Icmp_header;

private:
	inline void		ResetHeader();
	void			ResetPseudoHeader();

private:
};