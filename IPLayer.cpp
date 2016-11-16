#include "StdAfx.h"
#include "IPLayer.h"
#include "RouterDlg.h"

CIPLayer::CIPLayer(char* pName) : CBaseLayer(pName) 
{ 
	ResetHeader();
}

CIPLayer::~CIPLayer() 
{ 
}

void CIPLayer::SetDstIP(unsigned char* ip, int dev_num)
{
	if(dev_num == 1)
		memcpy(dev_1_dst_ip_addr, ip , 4 );
	else
		memcpy(dev_2_dst_ip_addr, ip , 4 );

	memcpy(&Ip_header.Ip_dstAddress,ip,4);
}

void CIPLayer::SetSrcIP(unsigned char* ip ,int dev_num)
{
	if(dev_num == 1)
		memcpy(dev_1_ip_addr , ip , 4 );
	else
		memcpy(dev_2_ip_addr , ip , 4 );

	memcpy(&Ip_header.Ip_srcAddress,ip,4);
}

void CIPLayer::SetSrcIPForRIPLayer(unsigned char* ip, int dev_num)
{
	if(dev_num == 1)
		memcpy(dev_1_ip_addr_for_rip , ip , 4 );
	else 
		memcpy(dev_2_ip_addr_for_rip , ip , 4 );
}

unsigned char* CIPLayer::GetDstIP(int dev_num)
{
	if(dev_num == 1)
		return dev_1_dst_ip_addr;
	return dev_2_dst_ip_addr;
}

unsigned char* CIPLayer::GetSrcIP(int dev_num)
{
	if(dev_num == 1)
		return dev_1_ip_addr;
	return dev_2_ip_addr;
}

unsigned char* CIPLayer::GetSrcIPForRIPLayer(int dev_num) 
{
	if(dev_num == 1)
		return dev_1_ip_addr_for_rip;
	return dev_2_ip_addr_for_rip;
}

unsigned char CIPLayer::GetProtocol(int dev_num) 
{
	if(dev_num == 1)
		return dev_1_protocol;
	return dev_2_protocol;
}

void CIPLayer::SetProtocol(unsigned char protocol, int dev_num)
{
	if(dev_num == 1)
		dev_1_protocol = protocol;
	else
		dev_2_protocol = protocol;

	Ip_header.Ip_protocol = protocol;
}

unsigned short CIPLayer::SetChecksum(unsigned char p_header[20])
{
	unsigned short word;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < IP_HEADER_SIZE; i = i + 2) {
		if(i == 10) 
			continue;
		word = ((p_header[i] << 8) & 0xFF00) + (p_header[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	sum = ~sum;
	return (unsigned short) sum;
}

BOOL CIPLayer::IsValidChecksum(unsigned char* p_header, unsigned short checksum)
{
	unsigned short word, ret;
	unsigned int sum = 0;
	int i;

	for(i = 0; i < IP_HEADER_SIZE; i = i + 2) {
		if(i == 10) continue;
		word = ((p_header[i] << 8) & 0xFF00) + (p_header[i+1] & 0xFF);
		sum = sum + (unsigned int) word;
	}

	while(sum >> 16)
		sum = (sum&0xFFFF) + (sum >> 16);

	ret = sum & checksum;
	return	ret == 0;
}

// ppayload == (unsigned char*)&Tcp_header
BOOL CIPLayer::Send(unsigned char* ppayload, int nlength, int dev_num)
{
	// IP 주소 셋팅은 Set 버튼을 눌렀을때 셋팅이 되었으므로 data와 전체 크기를 저장후 전송

	memcpy(Ip_header.Ip_data , ppayload , nlength);
	nlength = nlength + IP_HEADER_SIZE;
	Ip_header.Ip_len = (unsigned short) htons(nlength);
	memcpy(Ip_header.Ip_srcAddressByte, GetSrcIP(dev_num), 4);
	Ip_header.Ip_checksum = htons(SetChecksum((unsigned char*)&Ip_header));

	
	BOOL bSuccess = mp_UnderLayer->Send((unsigned char*) &Ip_header , nlength, dev_num);
	return bSuccess;
}

BOOL CIPLayer::Receive(unsigned char* ppayload, int dev_num)
{
	unsigned char broadcast[4] = { 0xff, 0xff, 0xff , 0xff };
	unsigned char multicast[4] = { 0xe0, 0, 0, 0x9 };
	
	CRouterDlg* routerDlg = ((CRouterDlg *) (GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	PIpHeader pFrame = (PIpHeader) ppayload;

	if(!memcmp(pFrame->Ip_srcAddressByte, GetSrcIP(dev_num), 4)) //자신이 보낸 패킷은 버린다
		return FALSE;

	if(!IsValidChecksum((unsigned char*) pFrame, ntohs(pFrame->Ip_checksum)))
		return FALSE;

	if( pFrame->Ip_timeToLive == 0 ) //ttl이 0일 경우 버림
      return FALSE;

	if (memcmp(pFrame->Ip_dstAddressByte, broadcast, 4) && memcmp(pFrame->Ip_dstAddressByte, multicast, 4)) { //broadcast, multicast가 아닐 경우
		if(memcmp(pFrame->Ip_dstAddressByte, GetSrcIP(1), 4) && memcmp(pFrame->Ip_dstAddressByte, GetSrcIP(2), 4)) { //router의 주소가 아닐 경우
			int selectIndex = Forwarding(pFrame->Ip_dstAddressByte);
		
			if (selectIndex != -1) { //routing 정보가 존재
				CRouterDlg::RoutingTable entry = CRouterDlg::route_table.GetAt(CRouterDlg::route_table.FindIndex(selectIndex));
				if (entry.metric == 16)
					return FALSE;
				if (entry.out_interface != dev_num) { //routing해야할 곳이 패킷이 들어온 방향과 다를 때
					unsigned char zeroip[4], destip[4];
					memcpy(destip, entry.nexthop, 4);
					memset(zeroip, 0, 4);
			
					if (memcmp(zeroip, destip, 4) == 0) //목표 network가 해당 router에 붙어있음
						SetDstIP(pFrame->Ip_dstAddressByte, entry.out_interface);
					else //목표 router가 한 hop 이상 떨어져있음
						SetDstIP(destip, entry.out_interface);
				
					pFrame->Ip_timeToLive = htons(ntohs(pFrame->Ip_timeToLive) - 1); // TTL 감소
					pFrame->Ip_checksum = htons(SetChecksum((unsigned char*) pFrame)); // Checksum 계산
					mp_UnderLayer->Send((unsigned char*) pFrame, (int) ntohs(pFrame->Ip_len), entry.out_interface);
					return TRUE;
				}
			}
			return FALSE;
		}
		
	}

	if (pFrame->Ip_protocol == 0x11) { // udp protocol (17) 확인
		SetSrcIPForRIPLayer(pFrame->Ip_srcAddressByte, dev_num);
		((CUDPLayer*)GetUpperLayer(0))->SetReceivePseudoHeader(pFrame->Ip_srcAddressByte, pFrame->Ip_dstAddressByte, (unsigned short) htons(ntohs(pFrame->Ip_len) - IP_HEADER_SIZE));
		return GetUpperLayer(0)->Receive((unsigned char *)pFrame->Ip_data, dev_num);
	}
		
	return FALSE;
}

void CIPLayer::ResetHeader( )
{
	Ip_header.Ip_version = 0x45; // version, header length
	Ip_header.Ip_typeOfService = 0x00;
	Ip_header.Ip_len = 0x0014; // 20byte default
	Ip_header.Ip_id = 0x0000;		
	Ip_header.Ip_fragmentOffset = 0x0000;
	Ip_header.Ip_timeToLive = 0x05; // ttl default
	Ip_header.Ip_protocol = 0x00; // setting
	Ip_header.Ip_checksum = 0x0000; 
	memset(Ip_header.Ip_srcAddressByte, 0, 4);
	memset(Ip_header.Ip_dstAddressByte, 0, 4);
	memset(Ip_header.Ip_data, 0, IP_MAX_DATA);
}

int CIPLayer::Forwarding(unsigned char destip[4]) 
{
	CRouterDlg::RoutingTable entry;
	int size = CRouterDlg::route_table.GetCount();
	unsigned char networkid[4];

	for(int index = 0; index < size; index++) {
		entry = CRouterDlg::route_table.GetAt(CRouterDlg::route_table.FindIndex(index));
		for(int i = 0; i < 4; i++)
			networkid[i] = destip[i] & entry.subnetmask[i];
		if(!memcmp(networkid, entry.ipAddress,  4)) 
			return index; // IP가 일치하는 Entry가 존재하면 그 index return.
	}
	return -1;
}
