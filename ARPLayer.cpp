#include "StdAfx.h"
#include "ARPLayer.h"
#include "EthernetLayer.h"
#include "IPLayer.h"
#include "RouterDlg.h"

CARPLayer::CARPLayer(char *pName) : CBaseLayer(pName)
{
	ResetMessage();
	buf_index = 0;
	out_index = 0;	
	buf[0].valid = 0;
	buf[1].valid = 1;
	//ResetCount();
}

CARPLayer::~CARPLayer(void)
{
}

BOOL CARPLayer::Send(unsigned char* ppayload, int nlength, int dev_num)
{
	int index;
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	unsigned char broadcast[4];
	unsigned char ether_broad[6];
	memset(broadcast,0xff,4);
	memset(ether_broad,0xff,6);

	if(memcmp(routerDlg->m_IPLayer->GetDstIP(dev_num), broadcast, 4) == 0) //broadcast일 경우 바로 보냄
		return ((CEthernetLayer*) this->mp_UnderLayer)->Send(ppayload,nlength, ip_type, dev_num);

	if((index = SearchIpAtTable(routerDlg->m_IPLayer->GetDstIP(dev_num))) != -1) { //search 결과 존재하는경우
		POSITION pos = Cache_Table.FindIndex(index);
		if(Cache_Table.GetAt(pos).cache_type == complete) { //해당 결과가 complete일 경우
			((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(Cache_Table.GetAt(pos).Mac_addr, dev_num); //해당 mac을 설정
			return ((CEthernetLayer*) this->mp_UnderLayer)->Send(ppayload, nlength, ip_type, dev_num);
		}
	}

	if(memcmp(routerDlg->m_IPLayer->GetDstIP(dev_num), routerDlg->GetSrcIP(dev_num), 4) == 0) { //자신 ip로 보내는 경우
		arp_message.arp_op = request; //request message
		memcpy(arp_message.arp_srcprotoaddr, routerDlg->GetSrcIP(dev_num), 4); //보내는 사람 ip
		memcpy(arp_message.arp_destprotoaddr, routerDlg->m_IPLayer->GetDstIP(dev_num), 4); //받는사람 ip
		memcpy(arp_message.arp_srchaddr, routerDlg->GetSrcMAC(dev_num), 6); //보내는 사람 mac
		((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(ether_broad, dev_num);
		return ((CEthernetLayer*) this->mp_UnderLayer)->Send((unsigned char *) &arp_message, ARP_MESSAGE_SIZE, arp_type, dev_num); //gratuitous arp message
	}

	//아무 정보가 없음, buffer에 패킷 저장
	ResetMessage(); //메시지 초기화
	if(buf[buf_index].valid == 0) { //버퍼가 비어있을 경우
		buf[buf_index].valid = 1;
		buf[buf_index].data = (unsigned char *) malloc(nlength);
		memcpy(buf[buf_index].data, ppayload, nlength); //패킷 저장
		buf[buf_index].dev_num = dev_num;
		buf[buf_index].nlength = nlength;
		memcpy(buf[buf_index].dest_ip, routerDlg->m_IPLayer->GetDstIP(dev_num), 4);
		buf_index++;
		buf_index %= 2; //Circular 버퍼
	} //안비어 있다면 패킷을버림

	arp_message.arp_op = request; //request message
	memcpy(arp_message.arp_srcprotoaddr, routerDlg->GetSrcIP(dev_num), 4); //보내는 사람 ip
	memcpy(arp_message.arp_srchaddr, ((CEthernetLayer*) this->mp_UnderLayer)->GetSourceAddress(dev_num), 6); //보내는사람 mac
	memcpy(arp_message.arp_destprotoaddr,routerDlg->m_IPLayer->GetDstIP(dev_num), 4); //받는사람 ip
	//LP_arpDlg->SetTimer(wait_timer,4000,NULL); //timer 가동
	((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(ether_broad, dev_num);
	return ((CEthernetLayer*)this->mp_UnderLayer)->Send((unsigned char *) &arp_message, ARP_MESSAGE_SIZE, arp_type, dev_num); //arp message
}

BOOL CARPLayer::Receive(unsigned char* ppayload,int dev_num) {
	LPARP_Message receive_arp_message = (LPARP_Message)ppayload;
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	unsigned char broadcast[4];
	unsigned char ether_broad[6];
	memset(broadcast,0xff,4);
	memset(ether_broad,0xff,6);
	ResetMessage();

	int index;
	if(receive_arp_message->arp_op == request) { //요구
		if(memcmp(receive_arp_message->arp_destprotoaddr, routerDlg->GetSrcIP(dev_num), 4)) { // 내 ip가 아닌데 올 경우
			if((index = SearchIpAtTable(receive_arp_message->arp_srcprotoaddr)) != -1) { //cache table에 존재할 경우 갱신, 그리고 Reply 보내지 않음
				//해당 entry를 찾아 값 수정
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
			} else { //없을 경우 -- ARP table에 추가
				LPCACHE_ENTRY Cache_entry;
				Cache_entry = (LPCACHE_ENTRY) malloc(sizeof(CACHE_ENTRY));
				memcpy(Cache_entry->Ip_addr, receive_arp_message->arp_srcprotoaddr, 4);
				memcpy(Cache_entry->Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_entry->cache_ttl = 1200; //20분
				Cache_entry->cache_type = complete;
				InsertCache(Cache_entry);
			}
		}
		else{ //자신의 ip로 온 경우 Update하고 Reply
			if(memcmp(receive_arp_message->arp_srchaddr, routerDlg->GetSrcMAC(dev_num), 6) == 0)
				return FALSE;	//자기자신 전송이므로 무시

			// srcmac이 다른 컴퓨터일 경우
			if(!memcmp(receive_arp_message->arp_srcprotoaddr, routerDlg->GetSrcIP(dev_num), 4)) //자신의 ip와 보내는 쪽 srcIP가 같은 경우 충돌
				AfxMessageBox("IP충돌 입니다.", 0, 0);

			if((index = SearchIpAtTable(receive_arp_message->arp_srcprotoaddr)) != -1) { //table에존재할 경우
				//해당 entry를 찾아 값 수정
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
			} else { //테이블에 존재하지 않을경우 해당 ip추가
				//arp_message 설정종료
				LPCACHE_ENTRY Cache_entry; //table 에 insert할 cache
				Cache_entry = (LPCACHE_ENTRY) malloc(sizeof(CACHE_ENTRY));
				memcpy(Cache_entry->Ip_addr, receive_arp_message->arp_srcprotoaddr, 4);
				memcpy(Cache_entry->Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_entry->cache_ttl = 1200; //20분
				Cache_entry->cache_type = complete;
				InsertCache(Cache_entry);
			}

			// Send reply message
			arp_message.arp_op = reply;
			memcpy(arp_message.arp_srchaddr, routerDlg->GetSrcMAC(dev_num), 6); //보내는 사람mac주소
			memcpy(arp_message.arp_srcprotoaddr, routerDlg->GetSrcIP(dev_num), 4); //보내는 사람ip주소
			memcpy(arp_message.arp_desthdaddr, receive_arp_message->arp_srchaddr, 6); //mac주소
			memcpy(arp_message.arp_destprotoaddr, receive_arp_message->arp_srcprotoaddr, 4); //ip주소
			((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(receive_arp_message->arp_srchaddr, dev_num);
			routerDlg->m_EthernetLayer->Send((unsigned char *) &arp_message,ARP_MESSAGE_SIZE,arp_type,dev_num);
		}

		return TRUE;
	} else if(receive_arp_message->arp_op == reply){ //응답
		if(!memcmp(receive_arp_message->arp_srcprotoaddr,routerDlg->GetSrcIP(dev_num),4)) //자신의 ip = 발송자 ip의경우
			AfxMessageBox("Ip충돌입니다.",0,0);

		else { //자신의 ip != 발송자 ip
			//arp_message 초기화 및 설정
			LPCACHE_ENTRY Cache_entry; //table 에 insert할 cache
			Cache_entry = (LPCACHE_ENTRY)malloc(sizeof(CACHE_ENTRY));
			memcpy(Cache_entry->Ip_addr,receive_arp_message->arp_srcprotoaddr,4);
			memcpy(Cache_entry->Mac_addr,receive_arp_message->arp_srchaddr,6);
			Cache_entry->cache_ttl = 1200; //20분
			Cache_entry->cache_type = complete;

			if((index = SearchIpAtTable(Cache_entry->Ip_addr)) != -1) { //존재할경우 값을교환
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, Cache_entry->Mac_addr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
				free(Cache_entry); //메모리 해제
			} else //존재하지 않을경우 테이블에 삽입
				InsertCache(Cache_entry); //cache insert

			// 얻은 Mac 주소를 통해 Buffer에 저장했던 data를 Send
			while(buf[out_index].valid == 1 && !memcmp(buf[out_index].dest_ip, receive_arp_message->arp_srcprotoaddr, 4)) { 
				//버퍼의자료 send
				routerDlg->m_EthernetLayer->SetDestinAddress(receive_arp_message->arp_srchaddr, dev_num);
				routerDlg->m_EthernetLayer->Send(buf[out_index].data,buf[out_index].nlength,ip_type,buf[out_index].dev_num);
				free(buf[out_index].data);
				buf[out_index].valid = 0;
				out_index--;
				out_index %= 2;
			}

			return TRUE;
		}
	}

	return FALSE;
}

int CARPLayer::SearchIpAtTable(unsigned char Ip_addr[4])
{
	int count, i, ret = -1;
	CACHE_ENTRY temp;

	count = Cache_Table.GetCount();
	for(i = 0; i < count; i++) {
		temp = Cache_Table.GetAt(Cache_Table.FindIndex(i));
		if(memcmp(Ip_addr, temp.Ip_addr, 4) == 0)
			ret = i;
	}
	return ret;	
}

BOOL CARPLayer::InsertCache(LPCACHE_ENTRY Cache_entry)
{
	Cache_Table.AddTail(*Cache_entry);
	this->updateCacheTable();
	free(Cache_entry);
	return TRUE;
}

BOOL CARPLayer::DeleteCache(int index)
{
	//Cache_Table.RemoveAt(index);
	return TRUE;
}

BOOL CARPLayer::DeleteAllCache()
{
	Cache_Table.RemoveAll();
	return TRUE;
}

BOOL CARPLayer::ResetMessage()
{
	arp_message.arp_hdtype = htons(0x0001);
	arp_message.arp_prototype = htons(0x0800);
	arp_message.arp_hdlength = 0x06;
	arp_message.arp_protolength = 0x04;
	arp_message.arp_op = htons(0x0000); //2개로 나뉨
	memset(arp_message.arp_srchaddr, 0, 6);
	memset(arp_message.arp_srcprotoaddr, 0, 4);
	memset(arp_message.arp_destprotoaddr, 0, 4);
	memset(arp_message.arp_desthdaddr, 0, 6);
	return TRUE;
}

//케쉬 테이블 업데이트 함수
void CARPLayer::updateCacheTable()
{ 
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	routerDlg->ListBox_ARPCacheTable.DeleteAllItems(); //내용 초기화
	CString ip,mac,time,type;
	POSITION index;
	CACHE_ENTRY entry; //head position

	for(int i = 0; i < Cache_Table.GetCount(); i++) { //케쉬 테이블 마지막까지
		index = Cache_Table.FindIndex(i);
		entry = Cache_Table.GetAt(index);
		ip.Format("%d.%d.%d.%d", entry.Ip_addr[0], entry.Ip_addr[1], entry.Ip_addr[2], entry.Ip_addr[3]);
		mac.Format("%x-%x-%x-%x-%x-%x", entry.Mac_addr[0], entry.Mac_addr[1], entry.Mac_addr[2], entry.Mac_addr[3], entry.Mac_addr[4], entry.Mac_addr[5]);
		(entry.cache_type == complete ? type.Format("Complete") : type.Format("Incomplete"));
		time.Format("%d:%d",entry.cache_ttl/60, entry.cache_ttl%60);
		routerDlg->ListBox_ARPCacheTable.InsertItem(i,NULL);
		routerDlg->ListBox_ARPCacheTable.SetItem(i,1,LVIF_TEXT,ip,0,0,0,NULL);
		routerDlg->ListBox_ARPCacheTable.SetItem(i,2,LVIF_TEXT,mac,0,0,0,NULL);
		routerDlg->ListBox_ARPCacheTable.SetItem(i,3,LVIF_TEXT,type,0,0,0,NULL);
		//routerDlg->ListBox_ARPCacheTable.SetItem(i,3,LVIF_TEXT,time,0,0,0,NULL);
	}

	routerDlg->ListBox_ARPCacheTable.UpdateWindow();
}

void CARPLayer::decreaseTime()
{
	if(!Cache_Table.IsEmpty()) {
		POSITION index;
		for(int i = 0; i < Cache_Table.GetCount(); i++) {
			index = Cache_Table.FindIndex(i);
			unsigned short ttl = Cache_Table.GetAt(index).cache_ttl -= 5;

			if(ttl <= 0) // 삭제
				Cache_Table.RemoveAt(index);
		}
		updateCacheTable();
	}
}

void CARPLayer::ResetCount()
{
	//sendCount = 0;
}
