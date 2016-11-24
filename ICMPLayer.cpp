#include "stdafx.h"
#include "ICMPLayer.h"
#include "RouterDlg.h"

CICMPLayer::CICMPLayer(char* pName) : CBaseLayer(pName) 
{ 
}

CICMPLayer::~CICMPLayer() 
{ 
}

BOOL CICMPLayer::Send(unsigned char* ppayload, int nlength, int dev_num) {
	return true;
}

BOOL CICMPLayer::Receive(unsigned char* ppayload, int dev_num) {
	PIcmpHeader pFrame = (PIcmpHeader) ppayload;
	CRouterDlg* routerDlg = ((CRouterDlg*) GetUpperLayer(0));

	if (dev_num == DEV_PUBLIC) { //incoming packet
		if (int index = searchTable(pFrame->Icmp_identifier, pFrame->Icmp_sequenceNumber) != -1) {
			routerDlg->m_IPLayer->SetDstPacketIP(Icmp_table.GetAt(Icmp_table.FindIndex(index)).inner_addr);
			//리스트에 존재하는 IP로 private network에 전송
			routerDlg->m_IPLayer->Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, DEV_PRIVATE);
		}
	}

	if (dev_num == DEV_PRIVATE) { //outgoing packet
		ICMP_ENTRY entry;
		entry.identifier = ntohs(pFrame->Icmp_identifier);
		memcpy(entry.inner_addr, routerDlg->m_IPLayer->GetSrcFromPacket(), 4);
		memcpy(entry.outer_addr, routerDlg->m_IPLayer->GetDstFromPacket(), 4);
		entry.sequenceNumber = ntohs(pFrame->Icmp_sequenceNumber);
		entry.time = 5;
		Icmp_table.AddTail(entry);

		routerDlg->m_IPLayer->SetSrcPacketIP(routerDlg->GetSrcIP(DEV_PUBLIC));
		routerDlg->m_IPLayer->Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, DEV_PUBLIC);
	}

	UpdateTable();
	return true;
}

int CICMPLayer::searchTable(unsigned short identifier, unsigned short sequenceNumber) {
	ICMP_ENTRY entry;
	int size = Icmp_table.GetCount();
	unsigned short id;
	unsigned short sequence;

	for(int index = 0; index < size; index++) {
		entry = Icmp_table.GetAt(Icmp_table.FindIndex(index));
		id = entry.identifier;
		sequence = entry.sequenceNumber;

		if (id == identifier && sequence == sequenceNumber) 
			return index;
	}
	return -1;
}

void CICMPLayer::UpdateTable()
{ 
	CRouterDlg* routerDlg = ((CRouterDlg*) GetUpperLayer(0));
	routerDlg->ListBox_ICMPTable.DeleteAllItems(); //내용 초기화
	CString inner_addr, outer_addr, identifier, sequenceNumber, time;
	ICMP_ENTRY entry; //head position

	for(int i = 0; i < Icmp_table.GetCount(); i++) { //케쉬 테이블 마지막까지
		entry = Icmp_table.GetAt(Icmp_table.FindIndex(i));

		inner_addr.Format("%d.%d.%d.%d", entry.inner_addr[0], entry.inner_addr[1], entry.inner_addr[2], entry.inner_addr[3]);
		outer_addr.Format("%d.%d.%d.%d", entry.outer_addr[0], entry.outer_addr[1], entry.outer_addr[2], entry.outer_addr[3]);
		identifier.Format("%d", entry.identifier);
		sequenceNumber.Format("%d", entry.sequenceNumber);
		time.Format("%d", entry.time);

		routerDlg->ListBox_ICMPTable.InsertItem(i, inner_addr);
		routerDlg->ListBox_ICMPTable.SetItem(i, 1, LVIF_TEXT, outer_addr, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 2, LVIF_TEXT, identifier, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 3, LVIF_TEXT, sequenceNumber, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 4, LVIF_TEXT, time, 0, 0, 0, NULL);
	}

	routerDlg->ListBox_ICMPTable.UpdateWindow();
}