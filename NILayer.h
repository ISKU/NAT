#pragma once
#include "baselayer.h"
#include "afxmt.h"

class CNILayer : public CBaseLayer  
{
public:
	CNILayer(char* pName);
	virtual ~CNILayer();
	CMutex mutex;
	void SetDevice(pcap_if_t *DeviceList,int dev_num);
	CWinThread* pThread_1;
	CWinThread* pThread_2;
	BOOL Send(unsigned char* ppayload, int nlength,int dev_num);
	BOOL Receive(unsigned char* ppayload) { return FALSE; }
	BOOL Receive(int dev_num);
	void StartReadThread();
	static unsigned int WaitPacket_1(LPVOID pParam);
	static unsigned int WaitPacket_2(LPVOID pParam);
	pcap_pkthdr *pkt_header[2];

private:
	pcap_if_t *DeviceList[2];
	pcap_t *Device[2];	
	unsigned char* pkt_data[2];
};