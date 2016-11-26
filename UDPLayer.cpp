#include "StdAfx.h"
#include "UDPLayer.h"
#include "RouterDlg.h"

CUDPLayer::CUDPLayer(char* pName) : CBaseLayer(pName)
{
	ResetPseudoHeader();
	circularIndex = 0;
}

CUDPLayer::~CUDPLayer()
{
}

void CUDPLayer::SetPseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length)
{
	memcpy(Udp_pseudo_header.Pseudo_srcIp, srcIp, 4);
	memcpy(Udp_pseudo_header.Pseudo_dstIp, dstIp, 4);
	Udp_pseudo_header.Pseudo_length = length;
}

unsigned short CUDPLayer::SetChecksum(int nlength) 
{
	unsigned char* p_pseudoheader = (unsigned char*) &Udp_pseudo_header;
	unsigned char* p_udpheader = (unsigned char*) receivedPacket;
	unsigned short word;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < UDP_PSEUDO_HEADER_SIZE; i = i + 2) {
		word = ((p_pseudoheader[i] << 8) & 0xFF00) + (p_pseudoheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	for(i = 0; i < nlength - (nlength % 2); i = i + 2){
		if(i == 6) continue;
		word = ((p_udpheader[i] << 8) & 0xFF00) + (p_udpheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	if(nlength % 2 == 1) {
		word = ((p_udpheader[nlength-1] << 8) & 0xFF00);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	sum = ~sum;
	return (unsigned short) sum;
}

BOOL CUDPLayer::IsValidChecksum(unsigned char* p_udpheader, unsigned short checksum, int nlength){
	unsigned char* p_pseudoheader = (unsigned char*) &Udp_pseudo_header;
	unsigned short word, ret;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < UDP_PSEUDO_HEADER_SIZE; i = i + 2) {
		word = ((p_pseudoheader[i] << 8) & 0xFF00) + (p_pseudoheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	for(i = 0; i < nlength - (nlength % 2); i = i + 2) {
		if(i == 6) continue;
		word = ((p_udpheader[i] << 8) & 0xFF00) + (p_udpheader[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	if(nlength%2 == 1) {
		word = ((p_udpheader[nlength-1] << 8) & 0xFF00);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	ret = sum & checksum;
	return ret == 0;
}

BOOL CUDPLayer::Send(unsigned char* ppayload, int nlength, int dev_num)
{
	return true;
}

BOOL CUDPLayer::Receive(unsigned char* ppayload, int dev_num)
{
	CRouterDlg* routerDlg =  ((CRouterDlg *) GetUpperLayer(0));
	CRouterDlg::NAT_ENTRY entry;
	receivedPacket = (PUdpHeader) ppayload;

	if ( !IsValidChecksum((unsigned char*) receivedPacket, ntohs(receivedPacket->Udp_checksum), ntohs(receivedPacket->Udp_length))) 
		return FALSE;

	if (dev_num == DEV_PUBLIC) { // incoming packet
		int index = SearchIncomingTable(ntohs(receivedPacket->Udp_dstPort));
		if (index != -1) {
			entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
			
			receivedPacket->Udp_dstPort = htons(entry.inner_port);
			routerDlg->m_IPLayer->SetDstPacketIP(entry.inner_addr);

			SetPseudoHeader(routerDlg->m_IPLayer->GetSrcFromPacket(), routerDlg->m_IPLayer->GetDstFromPacket(), receivedPacket->Udp_length);
			receivedPacket->Udp_checksum = (unsigned short) htons(SetChecksum(ntohs(receivedPacket->Udp_length)));
			routerDlg->m_IPLayer->Send(ppayload, UDP_HEADER_SIZE+UDP_MAX_DATA, DEV_PRIVATE);
		} else 
			return false;
	}

	if (dev_num == DEV_PRIVATE) { // outgoing packet
		int index = SearchOutgoingTable(routerDlg->m_IPLayer->GetSrcFromPacket(), ntohs(receivedPacket->Udp_srcPort));
		if (index == -1) {
			memcpy(entry.inner_addr, routerDlg->m_IPLayer->GetSrcFromPacket(), 4);
			entry.inner_port = ntohs(receivedPacket->Udp_srcPort);
			entry.outer_port = circularIndex + 49152;
			entry.status = 10;
			entry.time = 5;

			CRouterDlg::nat_table.AddTail(entry);

			receivedPacket->Udp_srcPort = htons(circularIndex + 49152);
			circularIndex = (circularIndex + 1) % 16383;
		} else {
			entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
			entry.time = 5;
			CRouterDlg::nat_table.SetAt(CRouterDlg::nat_table.FindIndex(index), entry);

			receivedPacket->Udp_srcPort = htons(entry.outer_port);
		}
		
		routerDlg->m_IPLayer->SetSrcPacketIP(routerDlg->GetSrcIP(DEV_PUBLIC));
		SetPseudoHeader(routerDlg->m_IPLayer->GetSrcFromPacket(), routerDlg->m_IPLayer->GetDstFromPacket(), receivedPacket->Udp_length);
		receivedPacket->Udp_checksum = (unsigned short) htons(SetChecksum(ntohs(receivedPacket->Udp_length)));
		routerDlg->m_IPLayer->Send(ppayload, UDP_HEADER_SIZE+UDP_MAX_DATA, DEV_PUBLIC);
	}

	routerDlg->UpdateNatTable();
	return true;
}

int CUDPLayer::SearchOutgoingTable(unsigned char inner_addr[4], unsigned short inner_port) {
	CRouterDlg::NAT_ENTRY entry;
	int size = CRouterDlg::nat_table.GetCount();

	for(int index = 0; index < size; index++) {
		entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
		if (!memcmp(entry.inner_addr, inner_addr, 4) && entry.inner_port == inner_port) 
			return index;
	}
	return -1;
}

int CUDPLayer::SearchIncomingTable(unsigned short outer_port) {
	CRouterDlg::NAT_ENTRY entry;
	int size = CRouterDlg::nat_table.GetCount();

	for(int index = 0; index < size; index++) {
		entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
		if (entry.outer_port == outer_port) 
			return index;
	}
	return -1;
}

void CUDPLayer::ResetPseudoHeader()
{
	Udp_pseudo_header.Pseudo_unused = 0;
	Udp_pseudo_header.Pseudo_protoId = 17;
}