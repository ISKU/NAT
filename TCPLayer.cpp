#include "StdAfx.h"
#include "TCPLayer.h"
#include "RouterDlg.h"

CTCPLayer::CTCPLayer(char* pName) : CBaseLayer(pName)
{
	ResetPseudoHeader();
}

CTCPLayer::~CTCPLayer()
{
}

void CTCPLayer::SetPseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length)
{
	memcpy(Tcp_pseudo_header.Pseudo_srcIp, srcIp, 4);
	memcpy(Tcp_pseudo_header.Pseudo_dstIp, dstIp, 4);
	Tcp_pseudo_header.Pseudo_length = length;
}

unsigned short CTCPLayer::SetChecksum(int nlength) 
{
	unsigned char* p_pseudoheader = (unsigned char*) &Tcp_pseudo_header;
	unsigned char* p_tcpheader = (unsigned char*) receivedPacket;
	unsigned short word;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < UDP_PSEUDO_HEADER_SIZE; i = i + 2) {
		word = ((p_pseudoheader[i] << 8) & 0xFF00) + (p_pseudoheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	for(i = 0; i < nlength - (nlength % 2); i = i + 2){
		if(i == 16) continue;
		word = ((p_tcpheader[i] << 8) & 0xFF00) + (p_tcpheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	if(nlength % 2 == 1) {
		word = ((p_tcpheader[nlength-1] << 8) & 0xFF00);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	sum = ~sum;
	return (unsigned short) sum;
}

BOOL CTCPLayer::IsValidChecksum(unsigned char* p_tcpheader, unsigned short checksum, int nlength){
	unsigned char* p_pseudoheader = (unsigned char*) &Tcp_pseudo_header;
	unsigned short word, ret;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < UDP_PSEUDO_HEADER_SIZE; i = i + 2) {
		word = ((p_pseudoheader[i] << 8) & 0xFF00) + (p_pseudoheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	for(i = 0; i < nlength - (nlength % 2); i = i + 2) {
		if(i == 16) continue;
		word = ((p_tcpheader[i] << 8) & 0xFF00) + (p_tcpheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	if(nlength%2 == 1) {
		word = ((p_tcpheader[nlength-1] << 8) & 0xFF00);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	ret = sum & checksum;
	return ret == 0;
}

BOOL CTCPLayer::Send(unsigned char* ppayload, int nlength, int dev_num)
{
	return true;
}

BOOL CTCPLayer::Receive(unsigned char* ppayload, int dev_num)
{
	CRouterDlg* routerDlg =  ((CRouterDlg *) GetUpperLayer(0));
	CRouterDlg::NAT_ENTRY entry;
	receivedPacket = (PTcpHeader) ppayload;

	if ( !IsValidChecksum((unsigned char*) receivedPacket, ntohs(receivedPacket->Tcp_checksum), routerDlg->m_IPLayer->GetLength())) 
		return FALSE;

	if (dev_num == DEV_PUBLIC) { // incoming packet
		int index = routerDlg->SearchIncomingTable(ntohs(receivedPacket->Tcp_dstPort));
		if (index != -1) {
			entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));

			if (entry.status == 15) {
				CRouterDlg::nat_table.RemoveAt(CRouterDlg::nat_table.FindIndex(index));
			}
			receivedPacket->Tcp_dstPort = htons(entry.inner_port);
			routerDlg->m_IPLayer->SetDstPacketIP(entry.inner_addr);

			SetPseudoHeader(routerDlg->m_IPLayer->GetSrcFromPacket(), routerDlg->m_IPLayer->GetDstFromPacket(), ntohs(routerDlg->m_IPLayer->GetLength()));
			receivedPacket->Tcp_checksum = (unsigned short) htons(SetChecksum(routerDlg->m_IPLayer->GetLength()));
			routerDlg->m_IPLayer->Send(ppayload, TCP_HEADER_SIZE+TCP_MAX_DATA, DEV_PRIVATE);
		} else 
			return false;
	}

	if (dev_num == DEV_PRIVATE) { // outgoing packet
		int index = routerDlg->SearchOutgoingTable(routerDlg->m_IPLayer->GetSrcFromPacket(), ntohs(receivedPacket->Tcp_srcPort));
		if (index == -1) {
			memcpy(entry.inner_addr, routerDlg->m_IPLayer->GetSrcFromPacket(), 4);
			entry.inner_port = ntohs(receivedPacket->Tcp_srcPort);
			entry.outer_port = routerDlg->GetCircularIndex();
			entry.status = 5;
			entry.time = 100;

			CRouterDlg::nat_table.AddTail(entry);
			receivedPacket->Tcp_srcPort = htons(entry.outer_port);
		} else {
			entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
			entry.time = 100;

			if ((receivedPacket->Tcp_flags & 0x01) == 0x01) // FIN
				entry.status = 15;

			CRouterDlg::nat_table.SetAt(CRouterDlg::nat_table.FindIndex(index), entry);
			receivedPacket->Tcp_srcPort = htons(entry.outer_port);
		}
		
		routerDlg->m_IPLayer->SetSrcPacketIP(routerDlg->GetSrcIP(DEV_PUBLIC));
		SetPseudoHeader(routerDlg->m_IPLayer->GetSrcFromPacket(), routerDlg->m_IPLayer->GetDstFromPacket(), ntohs(routerDlg->m_IPLayer->GetLength()));
		receivedPacket->Tcp_checksum = (unsigned short) htons(SetChecksum(routerDlg->m_IPLayer->GetLength()));
		routerDlg->m_IPLayer->Send(ppayload, TCP_HEADER_SIZE+TCP_MAX_DATA, DEV_PUBLIC);
	}

	routerDlg->UpdateNatTable();
	return true;
}

void CTCPLayer::ResetPseudoHeader()
{
	Tcp_pseudo_header.Pseudo_unused = 0;
	Tcp_pseudo_header.Pseudo_protoId = 0x06;
}
