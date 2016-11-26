#include "StdAfx.h"
#include "EthernetLayer.h"
#include "RouterDlg.h"

CEthernetLayer::CEthernetLayer(char* pName) : CBaseLayer(pName)
{
	ResetHeader();
}

CEthernetLayer::~CEthernetLayer()
{
}

// Header�� �ʱ�ȭ �ϴ� �Լ�.
void CEthernetLayer::ResetHeader()
{
	memset(Ethernet_Header.Ethernet_srcAddr.addr_ethernet, 0, 6); // destination address initialize
	memset(Ethernet_Header.Ethernet_dstAddr.addr_ethernet, 0, 6); // source address initialize
	Ethernet_Header.Ethernet_type = 0x0800; // data type initialize
	memset(Ethernet_Header.Ethernet_data, 0, ETHERNET_MAX_DATA); // data initialize
}

// �۽����� ethernet �ּҸ� ������ �Լ�.
unsigned char* CEthernetLayer::GetSourceAddress(int dev_num)
{
	if(dev_num == 1)
		return this->dev_1_mac_addr;
	return this->dev_2_mac_addr;
	// �ҽ� �ּҸ� ��� �Լ��̴�.
	// ������� m_sHeader�� �ҽ��ּҰ� ��� .enet_srcaddr�� �����Ѵ�.
}

// �������� ethernet �ּҸ� ������ �Լ�.
unsigned char* CEthernetLayer::GetDestinAddress()
{	
	return Ethernet_Header.Ethernet_dstAddr.addr_ethernet;
	//������ �ּҸ� ��� �Լ� �̴�.
	// �ɹ����� m_sHeader�� �������ּҰ� ��� .enet_dstaddr�� �����Ѵ�.
}

// �۽����� ethernet �ּҸ� �����ϴ� �Լ�.
void CEthernetLayer::SetSourceAddress(unsigned char *pAddress, int dev_num)
{
	// pAddress�� �Ѿ�� ���ڸ� memcpy �Ѵ�.
	if(dev_num == 1)
		memcpy(dev_1_mac_addr,pAddress, 6);
	else
		memcpy(dev_2_mac_addr,pAddress, 6);

	memcpy( Ethernet_Header.Ethernet_srcAddr.addr_ethernet, pAddress, 6);
}

// �������� ethernet �ּҸ� �����ϴ� �Լ�.
void CEthernetLayer::SetDestinAddress(unsigned char *pAddress, int dev_num)
{
	if(dev_num == 1)
		memcpy(dev_1_dst_mac_addr,pAddress,6);
	else
		memcpy(dev_2_dst_mac_addr,pAddress,6);

	memcpy( Ethernet_Header.Ethernet_dstAddr.addr_ethernet, pAddress, 6) ;
	// �ɹ� ������ ������ �ּҸ� �ִ� enet_dstaddr�� 
	//���ڷ� ���� paddress�� ���� �����Ѵ�.
}

// �׷��� �Ű������� �Ѿ���� ppayload�� ������ test packet�� �Է��� "Group 6 test packet" �̶�� ���ڿ� �Դϴ�.
BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength, int dev_num)
{
	return true;
}

// NILayer���� data�� ������ demultiplexing �ϰ� ���� �������� �����ִ� �������� ���� �ϸ� �ɰͰ����ϴ�.
BOOL CEthernetLayer::Receive(unsigned char* ppayload, int dev_num)
{
	CRouterDlg* routerDlg = ((CRouterDlg *) (GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	PEthernetHeader pFrame = (PEthernetHeader) ppayload;
	char Broad[6];
	memset(Broad, 0xff, 6);
	
	if(!memcmp(&pFrame->Ethernet_srcAddr,GetSourceAddress(dev_num),6)) //�ڽſ��� ���� ��Ŷ
		return FALSE;
	
	if (dev_num == DEV_PUBLIC) {
		if ((!memcmp(&pFrame->Ethernet_dstAddr, routerDlg->GetSrcMAC(dev_num), 6)) || (!memcmp(&pFrame->Ethernet_dstAddr, Broad, 6))) {
			if(pFrame->Ethernet_type == arp_type) //arp_type�� ���
				GetUpperLayer(1)->Receive((unsigned char*) pFrame->Ethernet_data, dev_num);
			else if(pFrame->Ethernet_type == ip_type) //ip_type�� ���
				GetUpperLayer(0)->Receive((unsigned char*) pFrame->Ethernet_data, dev_num);
		}
	}

	if (dev_num == DEV_PRIVATE) {
		if ((!memcmp(&pFrame->Ethernet_dstAddr, routerDlg->GetSrcMAC(dev_num), 6)) || (!memcmp(&pFrame->Ethernet_dstAddr, Broad, 6))) {
			if(pFrame->Ethernet_type == arp_type) //arp_type�� ���
				GetUpperLayer(1)->Receive((unsigned char*) pFrame->Ethernet_data, dev_num);
			else if(pFrame->Ethernet_type == ip_type) //ip_type�� ���
				GetUpperLayer(0)->Receive((unsigned char*) pFrame->Ethernet_data, dev_num);
		}
	}

	return true;
}

void CEthernetLayer::SetType(unsigned short type)
{
	Ethernet_Header.Ethernet_type = type;
}

BOOL CEthernetLayer::Send(unsigned char* ppayload, int nlength, unsigned short type, int dev_num)
{
	CRouterDlg* routerDlg = ((CRouterDlg *) (GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	memcpy(Ethernet_Header.Ethernet_data, ppayload, nlength);
	memcpy(&Ethernet_Header.Ethernet_srcAddr, routerDlg->GetSrcMAC(dev_num), 6);
	memcpy(&Ethernet_Header.Ethernet_dstAddr, GetDestinAddress(), 6);
	Ethernet_Header.Ethernet_type = type;

	if (Ethernet_Header.Ethernet_type == arp_type)
		mp_UnderLayer->Send((unsigned char*) &Ethernet_Header, nlength + ETHERNET_HEADER_SIZE, dev_num);
	else if (Ethernet_Header.Ethernet_type == ip_type)
		mp_UnderLayer->Send((unsigned char*) &Ethernet_Header, nlength + ETHERNET_HEADER_SIZE, dev_num);

	return true;
}