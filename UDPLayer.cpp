#include "StdAfx.h"
#include "UDPLayer.h"
#include "RouterDlg.h"

CUDPLayer::CUDPLayer(char* pName) : CBaseLayer(pName)
{
	ResetHeader();
	ResetPseudoHeader();
}

CUDPLayer::~CUDPLayer()
{
}

unsigned short CUDPLayer::GetSrcPort()
{
	return Udp_header.Udp_srcPort;
}

unsigned short CUDPLayer::GetDstPort()
{
	return Udp_header.Udp_dstPort;
}

unsigned short CUDPLayer::GetLength(int dev_num)
{
	if(dev_num == 1)
		return dev_1_length;
	return dev_2_length;
}

void CUDPLayer::SetSrcPort(unsigned short port)
{
	Udp_header.Udp_srcPort = port;
}

void CUDPLayer::SetDstPort(unsigned short port)
{
	Udp_header.Udp_dstPort = port;
}

void CUDPLayer::SetLength(unsigned short length, int dev_num)
{
	if(dev_num == 1)
		dev_1_length = length;
	else
		dev_2_length = length;

	Udp_header.Udp_length = length;
}

void CUDPLayer::SetLengthForRIP(unsigned short length, int dev_num)
{
	if(dev_num == 1)
		dev_1_length_for_rip = length;
	else
		dev_2_length_for_rip = length;
}

unsigned short CUDPLayer::GetLengthForRIP(int dev_num)
{
	if(dev_num == 1)
		return dev_1_length_for_rip;
	return dev_2_length_for_rip;
}

void CUDPLayer::SetSendPseudoHeader(unsigned short length, int dev_num)
{
	CRouterDlg * routerDlg =  ((CRouterDlg *) GetUpperLayer(0));

	memcpy(Udp_pseudo_header.Pseudo_srcIp, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4);
	memcpy(Udp_pseudo_header.Pseudo_dstIp, routerDlg->m_IPLayer->GetDstIP(dev_num), 4);
	Udp_pseudo_header.Pseudo_length = length;
}

void CUDPLayer::SetReceivePseudoHeader(unsigned char* srcIp, unsigned char* dstIp, unsigned short length)
{
	CRouterDlg * routerDlg =  ((CRouterDlg *) GetUpperLayer(0));

	memcpy(Udp_pseudo_header.Pseudo_srcIp, srcIp, 4);
	memcpy(Udp_pseudo_header.Pseudo_dstIp, dstIp, 4);
	Udp_pseudo_header.Pseudo_length = length;
}

unsigned short CUDPLayer::SetChecksum(int nlength) 
{
	unsigned char* p_pseudoheader = (unsigned char*) &Udp_pseudo_header;
	unsigned char* p_udpheader = (unsigned char*) &Udp_header;
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
	/*
	CRouterDlg * routerDlg =  ((CRouterDlg *) GetUpperLayer(0));

	memcpy(Udp_header.Udp_data, ppayload, nlength);
	nlength = UDP_HEADER_SIZE + nlength;
	routerDlg->m_IPLayer->SetProtocol(0x11, dev_num);

	Udp_header.Udp_length = (unsigned short) htons(nlength);
	SetSendPseudoHeader((unsigned short) htons(nlength), dev_num);
	Udp_header.Udp_checksum = (unsigned short) htons(SetChecksum(nlength));

	BOOL bSuccess = mp_UnderLayer->Send((unsigned char*)&Udp_header, nlength, dev_num);
	*/
	return true;
	
}

BOOL CUDPLayer::Receive(unsigned char* ppayload, int dev_num)
{
	/*
	PUdpHeader pFrame = (PUdpHeader) ppayload;
	BOOL bSuccess = FALSE;

	if ( !IsValidChecksum((unsigned char*) pFrame, ntohs(pFrame->Udp_checksum), ntohs(pFrame->Udp_length))) 
		return FALSE;

	if (pFrame->Udp_dstPort == 0x0802) { // check dst port 520
		SetLengthForRIP((unsigned short) htons(pFrame->Udp_length), dev_num);
		bSuccess = GetUpperLayer(0)->Receive((unsigned char *) pFrame->Udp_data, dev_num);
	}
	*/
	return true;
}

void CUDPLayer::ResetHeader()
{
	Udp_header.Udp_srcPort = 0x0802; // int 520 
	Udp_header.Udp_dstPort = 0x0802; // int 520
	Udp_header.Udp_length = 0x8; // header + data length (default udp header length)
	Udp_header.Udp_checksum = 0x00; // checksum
}

void CUDPLayer::ResetPseudoHeader()
{
	Udp_pseudo_header.Pseudo_unused = 0;
	Udp_pseudo_header.Pseudo_protoId = 17;
}