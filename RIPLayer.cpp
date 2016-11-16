#include "StdAfx.h"
#include "RIPLayer.h"
#include "RouterDlg.h"

CRIPLayer::CRIPLayer(char* pName) : CBaseLayer(pName)
{
	ResetHeader();
}

CRIPLayer::~CRIPLayer()
{
}

BOOL CRIPLayer::Send(int command, int dev_num, int resend)
{
	unsigned char broadcast[4];
	memset(broadcast, 0xff, 4);
	unsigned char macbroadcast[6];
	memset(macbroadcast, 0xff, 6);

	CRouterDlg * routerDlg = ((CRouterDlg *) GetUpperLayer(0));
	int messageLength;

	if (command == 1) {
		Rip_header.Rip_command = 0x01;
		CreateRequestMessage();
		messageLength = 20; // Request �� ��쿡�� �Ѱ���
	
		//resand == 1�� ��찡 ���� ���� resand�� ���� ���ǽ��� �ʿ� ����.
		routerDlg->m_IPLayer->SetDstIP(broadcast, dev_num);
		routerDlg->m_EthernetLayer->SetDestinAddress(macbroadcast, dev_num);

		routerDlg->m_UDPLayer->SetSrcPort(0x0802); // 520(UDP)
		return mp_UnderLayer->Send((unsigned char*) &Rip_header, RIP_HEADER_SIZE + messageLength, dev_num);
	}

	if (command == 2) {
		Rip_header.Rip_command = 0x02;
		messageLength = CreateResponseMessageTable(dev_num) * 20;// Response�� ��쿡�� Routing table�� Entry ������ŭ (4Byte * 5��)
	
		
		if(resend == 0){  
			routerDlg->m_IPLayer->SetDstIP(broadcast, dev_num);
			routerDlg->m_EthernetLayer->SetDestinAddress(macbroadcast, dev_num);
		}

		routerDlg->m_UDPLayer->SetSrcPort(0x0802); // 520(UDP)
		BOOL bSuccess = mp_UnderLayer->Send((unsigned char*) &Rip_header, RIP_HEADER_SIZE + messageLength, dev_num);
	}

	return false;
}

BOOL CRIPLayer::Receive(unsigned char* ppayload, int dev_num)
{
	CRouterDlg* routerDlg = ((CRouterDlg *) GetUpperLayer(0));
	PRipHeader pFrame = (PRipHeader) ppayload;
	unsigned char netmask[4] = { 0xff, 0xff, 0xff , 0 };

	// ���� Packet���� RIP Message�� �Ǹ� Entry�� ����(UDP ��ü ���̿��� UDP header(8), RIP �� ����(4) �� ����)
	unsigned short length = routerDlg->m_UDPLayer->GetLengthForRIP(dev_num) - 12;

	if (pFrame->Rip_command == 0x01) { // command : Request�� ���� ���, command�� Response�� �����Ͽ� �ٽ� ����
		routerDlg->m_IPLayer->SetDstIP(routerDlg->m_IPLayer->GetSrcIPForRIPLayer(dev_num), dev_num);
		routerDlg->m_EthernetLayer->SetDestinAddress(routerDlg->m_EthernetLayer->GetSourceAddressForRip(dev_num),dev_num);// ���� ������� ���ּҷ� ��������
		Send(2, dev_num , 1);
	}

	if (pFrame->Rip_command == 0x02) { // command : Response�� ���� ���, Routing table ������Ʈ
		int numOfEntries = length / 20; // ���� Packet���� RIP Message�� �Ǹ� Entry�� ����(Entry �� ���̰� 20)

		for (int index = 0; index < numOfEntries; index++) {
			CRouterDlg::RoutingTable entry;
			unsigned int metric = htonl(pFrame->Rip_table[index].Rip_metric);
			int selectIndex = ContainsRouteTableEntry(pFrame->Rip_table[index].Rip_ipAddress);

			if (selectIndex != -1) { 	// �ش� IP�� �����Ѵٸ� ���Ͽ� Update
				entry = CRouterDlg::route_table.GetAt(CRouterDlg::route_table.FindIndex(selectIndex));
				
				// routerDlg->m_IPLayer->GetSrcIP(2) : next-hop

				// next-hop�� ���� ���
				if (!memcmp(entry.nexthop, routerDlg->m_IPLayer->GetSrcIPForRIPLayer(dev_num), 4)) { // next-hop�� ���� ���
					if (metric == 16 && entry.status != 2) {
						entry.metric = metric;
						entry.status = 2;
						entry.time = 5;
					} else if (metric == 16 && entry.status == 2) {
						;
					} else {
						entry.metric = metric;
						entry.status = 1;
						entry.time = 10;
					}
					CRouterDlg::route_table.SetAt(CRouterDlg::route_table.FindIndex(selectIndex), entry);
				} else { // next-hop�� �ٸ� ���
					if (metric == 16)
						continue;
					if (metric < entry.metric) { // ���ο� metric���� �� ������ �װɷ� Update
						entry.metric = metric;
						entry.out_interface = dev_num;
						memcpy(&entry.nexthop, routerDlg->m_IPLayer->GetSrcIPForRIPLayer(dev_num), 4);
						entry.status = 1;
						entry.time = 10;
						CRouterDlg::route_table.SetAt(CRouterDlg::route_table.FindIndex(selectIndex), entry);
					} else { // �̹� �����ϴ� metric���� �� ������ �״�� ��
						;
					}
				}
			} else { // �ش� IP�� �������� ������ �״�� Routing table�� �߰�
				if (metric == 16)
					continue;
				for(int i = 0; i < 4; i++)
					entry.ipAddress[i] = pFrame->Rip_table[index].Rip_ipAddress[i];
				memcpy(entry.subnetmask, pFrame->Rip_table[index].Rip_subnetmask, 4);
				entry.metric = metric;
				entry.out_interface = dev_num;
				memcpy(&entry.nexthop, routerDlg->m_IPLayer->GetSrcIPForRIPLayer(dev_num), 4);
				entry.status = 1;
				entry.time = 10;
				CRouterDlg::route_table.AddTail(entry);
			}
		}

		routerDlg->UpdateRouteTable();
	}

	return true;
}

void CRIPLayer::ResetHeader()
{
	Rip_header.Rip_command = 0x01; // request: 1, response: 2 
	Rip_header.Rip_version = 0x02; // version: 2
	Rip_header.Rip_reserved = 0x0000; // must be zero
}

void CRIPLayer::CreateRequestMessage()
{
	Rip_header.Rip_table[0].Rip_family = 0x0200;
	Rip_header.Rip_table[0].Rip_tag = 0x0100;
	memset(Rip_header.Rip_table[0].Rip_ipAddress,  0, 4);
	memset(Rip_header.Rip_table[0].Rip_subnetmask, 0, 4);
	memset(Rip_header.Rip_table[0].Rip_nexthop, 0, 4);
	Rip_header.Rip_table[0].Rip_metric = htonl(16); // Default metric : 16
}

int CRIPLayer::CreateResponseMessageTable(int dev_num)
{
	CRouterDlg::RoutingTable entry;
	int entries = CRouterDlg::route_table.GetCount();
	int length = 0; 

	for (int index = 0; index < entries; index++) {
		entry = CRouterDlg::route_table.GetAt(CRouterDlg::route_table.FindIndex(index));

		if (entry.out_interface != dev_num) {
			Rip_header.Rip_table[length].Rip_family = 0x0200;
			Rip_header.Rip_table[length].Rip_tag = 0x0100;
			memcpy(Rip_header.Rip_table[length].Rip_ipAddress, entry.ipAddress, 4);
			memcpy(Rip_header.Rip_table[length].Rip_subnetmask, entry.subnetmask, 4);
			memset(Rip_header.Rip_table[length].Rip_nexthop, 0, 4);
			if(entry.metric == INFINITY_HOP_COUNT)
				Rip_header.Rip_table[length].Rip_metric = htonl(entry.metric);
			else
				Rip_header.Rip_table[length].Rip_metric = htonl(entry.metric + 1); // Metric�� 1���� ������
			length++;
		}
	}

	return length;
}

// Route Table�� �ش� Entry�� �����ϴ��� Ȯ��, index�� ��ȯ�Ѵ�.
int CRIPLayer::ContainsRouteTableEntry(unsigned char Ip_addr[4]) 
{
	CRouterDlg::RoutingTable entry;
	int size = CRouterDlg::route_table.GetCount();

	if (size != 0) {
		for(int index = 0; index < size; index++) {
			entry = CRouterDlg::route_table.GetAt(CRouterDlg::route_table.FindIndex(index));
			if(!memcmp(Ip_addr, entry.ipAddress,  4)) 
				return index; // IP�� ��ġ�ϴ� Entry�� �����ϸ� �� index return.
		}
	}

	return -1;
}