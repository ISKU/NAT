#include "stdafx.h"
#include "ICMPLayer.h"
#include "RouterDlg.h"

BOOL CICMPLayer::Send(unsigned char* ppayload, int nlength, int dev_num) {
}

BOOL CICMPLayer::Receive(unsigned char* ppayload, int dev_num) {
	PIcmpHeader pFrame = (PIcmpHeader) ppayload;
	CRouterDlg* routerDlg = ((CRouterDlg*) GetUpperLayer(0));

	if (dev_num == 1) { //incoming packet
		if (int index = ExistIdentifier(pFrame->Icmp_identifier) != -1) {
			routerDlg->m_IPLayer->SetDstIP(Icmp_table.GetAt(Icmp_table.FindIndex(index)).inner_addr, 2);
			//����Ʈ�� �����ϴ� IP�� private network�� ����
			Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, 2);
		}
	} else if (dev_num == 2) { //outgoing packet
		if (ExistIdentifier(pFrame->Icmp_identifier) != -1) {
			//����Ʈ ���� ���� �� ����?, 
		} else {
		
		}
		Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, 1);
	}
}

int CICMPLayer::ExistIdentifier(unsigned short identifier) {
	ICMP_ENTRY entry;
	int size = Icmp_table.GetCount();
	unsigned short id;

	for(int index = 0; index < size; index++) {
		entry = Icmp_table.GetAt(Icmp_table.FindIndex(index));
		id = entry.identifier;
		if (id == identifier) 
			return index;
	}
	return -1;
}