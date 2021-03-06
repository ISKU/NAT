#include "StdAfx.h"
#include "stdafx.h"
#include "ICMPLayer.h"
#include "RouterDlg.h"

CList<CICMPLayer::ICMP_ENTRY, CICMPLayer::ICMP_ENTRY&> CICMPLayer::Icmp_table;

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
		int index = searchTable(ntohs(pFrame->Icmp_identifier), ntohs(pFrame->Icmp_sequenceNumber));
		if (index != -1) {
			ICMP_ENTRY entry = Icmp_table.GetAt(Icmp_table.FindIndex(index));
			entry.time = 0;
			Icmp_table.SetAt(Icmp_table.FindIndex(index), entry);
			routerDlg->m_IPLayer->SetDstPacketIP(Icmp_table.GetAt(Icmp_table.FindIndex(index)).inner_addr);
			routerDlg->m_IPLayer->Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, DEV_PRIVATE);
		}
	}

	if (dev_num == DEV_PRIVATE) { //outgoing packet
		if (searchTable(pFrame->Icmp_identifier, pFrame->Icmp_sequenceNumber) == -1) {
			ICMP_ENTRY entry;
			entry.identifier = ntohs(pFrame->Icmp_identifier);
			memcpy(entry.inner_addr, routerDlg->m_IPLayer->GetSrcFromPacket(), 4);
			memcpy(entry.outer_addr, routerDlg->m_IPLayer->GetDstFromPacket(), 4);
			entry.sequenceNumber = ntohs(pFrame->Icmp_sequenceNumber);
			entry.time = 2;
			Icmp_table.AddTail(entry);	
		}
		routerDlg->m_IPLayer->SetSrcPacketIP(routerDlg->GetSrcIP(DEV_PUBLIC));
		routerDlg->m_IPLayer->Send(ppayload, ICMP_HEADER_SIZE+ICMP_MAX_DATA, DEV_PUBLIC);		
	}

	UpdateTable();
	return true;
}

int CICMPLayer::searchTable(unsigned short identifier, unsigned short sequenceNumber) {
	ICMP_ENTRY entry;

	for(int index = 0; index < Icmp_table.GetCount(); index++) {
		entry = Icmp_table.GetAt(Icmp_table.FindIndex(index));
		if (entry.identifier == identifier && entry.sequenceNumber == sequenceNumber) 
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

		routerDlg->ListBox_ICMPTable.InsertItem(i, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 1, LVIF_TEXT, inner_addr, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 2, LVIF_TEXT, outer_addr, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 3, LVIF_TEXT, identifier, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 4, LVIF_TEXT, sequenceNumber, 0, 0, 0, NULL);
		routerDlg->ListBox_ICMPTable.SetItem(i, 5, LVIF_TEXT, time, 0, 0, 0, NULL);
	}

	routerDlg->ListBox_ICMPTable.UpdateWindow();
}

unsigned int CICMPLayer::IcmpTableCheck(LPVOID pParam) {
	ICMP_ENTRY entry;

	while(1) {
		for (int index = 0; index < Icmp_table.GetCount(); index++) {
			entry = Icmp_table.GetAt(Icmp_table.FindIndex(index));

			if (entry.time == 0) {
				Icmp_table.RemoveAt(Icmp_table.FindIndex(index));
				index--;
			} else {
				entry.time = entry.time - 1;
				Icmp_table.SetAt(Icmp_table.FindIndex(index), entry);
			}	
		}

		((CICMPLayer*)pParam)->UpdateTable();
		Sleep(5000);
	}

	return 0;
}

void CICMPLayer::StartReadThread()
{
	pThread_1 = AfxBeginThread(IcmpTableCheck, this);

	if(pThread_1 == NULL)
		AfxMessageBox("Read 쓰레드 생성 실패");
}