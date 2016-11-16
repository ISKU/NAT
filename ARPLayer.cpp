#include "StdAfx.h"
#include "ARPLayer.h"
#include "EthernetLayer.h"
#include "IPLayer.h"
#include "RouterDlg.h"

CARPLayer::CARPLayer(char *pName) : CBaseLayer(pName)
{
	ResetMessage();
	ResetMessageProxy();
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
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	unsigned char broadcast[4];
	unsigned char ether_broad[6];
	memset(broadcast,0xff,4);
	memset(ether_broad,0xff,6);

	if(memcmp(routerDlg->m_IPLayer->GetDstIP(dev_num), broadcast, 4) == 0) //broadcast�� ��� �ٷ� ����
		return ((CEthernetLayer*) this->mp_UnderLayer)->Send(ppayload,nlength, ip_type, dev_num);

	if((index = SearchIpAtTable(routerDlg->m_IPLayer->GetDstIP(dev_num))) != -1) { //search ��� �����ϴ°��
		POSITION pos = Cache_Table.FindIndex(index);
		if(Cache_Table.GetAt(pos).cache_type == complete) { //�ش� ����� complete�� ���
			((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(Cache_Table.GetAt(pos).Mac_addr, dev_num); //�ش� mac�� ����
			return ((CEthernetLayer*) this->mp_UnderLayer)->Send(ppayload, nlength, ip_type, dev_num);
		}
	}

	if(memcmp(routerDlg->m_IPLayer->GetDstIP(dev_num), routerDlg->m_IPLayer->GetSrcIP(dev_num), 4) == 0) { //�ڽ� ip�� ������ ���
		arp_message.arp_op = request; //request message
		memcpy(arp_message.arp_srcprotoaddr, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4); //������ ��� ip
		memcpy(arp_message.arp_destprotoaddr, routerDlg->m_IPLayer->GetDstIP(dev_num), 4); //�޴»�� ip
		memcpy(arp_message.arp_srchaddr, routerDlg->m_EthernetLayer->GetSourceAddress(dev_num), 6); //������ ��� mac
		((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(ether_broad, dev_num);
		return ((CEthernetLayer*) this->mp_UnderLayer)->Send((unsigned char *) &arp_message, ARP_MESSAGE_SIZE, arp_type, dev_num); //gratuitous arp message
	}

	//�ƹ� ������ ����, buffer�� ��Ŷ ����
	ResetMessage(); //�޽��� �ʱ�ȭ
	if(buf[buf_index].valid == 0) { //���۰� ������� ���
		buf[buf_index].valid = 1;
		buf[buf_index].data = (unsigned char *) malloc(nlength);
		memcpy(buf[buf_index].data, ppayload, nlength); //��Ŷ ����
		buf[buf_index].dev_num = dev_num;
		buf[buf_index].nlength = nlength;
		memcpy(buf[buf_index].dest_ip, routerDlg->m_IPLayer->GetDstIP(dev_num), 4);
		buf_index++;
		buf_index %= 2; //Circular ����
	} //�Ⱥ�� �ִٸ� ��Ŷ������

	arp_message.arp_op = request; //request message
	memcpy(arp_message.arp_srcprotoaddr, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4); //������ ��� ip
	memcpy(arp_message.arp_srchaddr, ((CEthernetLayer*) this->mp_UnderLayer)->GetSourceAddress(dev_num), 6); //�����»�� mac
	memcpy(arp_message.arp_destprotoaddr,routerDlg->m_IPLayer->GetDstIP(dev_num), 4); //�޴»�� ip
	//LP_arpDlg->SetTimer(wait_timer,4000,NULL); //timer ����
	((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(ether_broad, dev_num);
	return ((CEthernetLayer*)this->mp_UnderLayer)->Send((unsigned char *) &arp_message, ARP_MESSAGE_SIZE, arp_type, dev_num); //arp message
}

BOOL CARPLayer::Receive(unsigned char* ppayload,int dev_num) {
	LPARP_Message receive_arp_message = (LPARP_Message)ppayload;
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	unsigned char broadcast[4];
	unsigned char ether_broad[6];
	memset(broadcast,0xff,4);
	memset(ether_broad,0xff,6);
	ResetMessage();

	int index;
	if(receive_arp_message->arp_op == request) { //�䱸
		if(memcmp(receive_arp_message->arp_destprotoaddr, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4)) { // �� ip�� �ƴѵ� �� ���
			if((index = SearchProxyTable(receive_arp_message->arp_destprotoaddr)) != -1) 	{ //proxy Table�� ������ ���
				POSITION pos = Proxy_Table.FindIndex(index);
				PROXY_ENTRY entry = Proxy_Table.GetAt(pos);
				proxy_arp_message.arp_op = reply;
				memcpy(proxy_arp_message.arp_destprotoaddr, receive_arp_message->arp_srcprotoaddr, 4);
				memcpy(proxy_arp_message.arp_desthdaddr, receive_arp_message->arp_srchaddr, 6);
				memcpy(proxy_arp_message.arp_srchaddr, routerDlg->m_EthernetLayer->GetSourceAddress(dev_num), 6);
				memcpy(proxy_arp_message.arp_srcprotoaddr, entry.Ip_addr, 4); //proxy ���� �ְ� ���۽�����
				((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(ether_broad, dev_num);
				routerDlg->m_EthernetLayer->Send((unsigned char *) &proxy_arp_message, ARP_MESSAGE_SIZE, arp_type,dev_num);
			}

			if((index = SearchIpAtTable(receive_arp_message->arp_srcprotoaddr)) != -1) { //cache table�� ������ ��� ����, �׸��� Reply ������ ����
				//�ش� entry�� ã�� �� ����
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
			} else { //���� ��� -- ARP table�� �߰�
				LPCACHE_ENTRY Cache_entry;
				Cache_entry = (LPCACHE_ENTRY) malloc(sizeof(CACHE_ENTRY));
				memcpy(Cache_entry->Ip_addr, receive_arp_message->arp_srcprotoaddr, 4);
				memcpy(Cache_entry->Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_entry->cache_ttl = 1200; //20��
				Cache_entry->cache_type = complete;
				InsertCache(Cache_entry);
			}
		}
		else{ //�ڽ��� ip�� �� ��� Update�ϰ� Reply
			if(memcmp(receive_arp_message->arp_srchaddr, routerDlg->m_EthernetLayer->GetSourceAddress(dev_num), 6) == 0)
				return FALSE;	//�ڱ��ڽ� �����̹Ƿ� ����

			// srcmac�� �ٸ� ��ǻ���� ���
			if(!memcmp(receive_arp_message->arp_srcprotoaddr, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4)) //�ڽ��� ip�� ������ �� srcIP�� ���� ��� �浹
				AfxMessageBox("IP�浹 �Դϴ�.", 0, 0);

			if((index = SearchIpAtTable(receive_arp_message->arp_srcprotoaddr)) != -1) { //table�������� ���
				//�ش� entry�� ã�� �� ����
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
			} else { //���̺� �������� ������� �ش� ip�߰�
				//arp_message ��������
				LPCACHE_ENTRY Cache_entry; //table �� insert�� cache
				Cache_entry = (LPCACHE_ENTRY) malloc(sizeof(CACHE_ENTRY));
				memcpy(Cache_entry->Ip_addr, receive_arp_message->arp_srcprotoaddr, 4);
				memcpy(Cache_entry->Mac_addr, receive_arp_message->arp_srchaddr, 6);
				Cache_entry->cache_ttl = 1200; //20��
				Cache_entry->cache_type = complete;
				InsertCache(Cache_entry);
			}

			// Send reply message
			arp_message.arp_op = reply;
			memcpy(arp_message.arp_srchaddr, routerDlg->m_EthernetLayer->GetSourceAddress(dev_num), 6); //������ ���mac�ּ�
			memcpy(arp_message.arp_srcprotoaddr, routerDlg->m_IPLayer->GetSrcIP(dev_num), 4); //������ ���ip�ּ�
			memcpy(arp_message.arp_desthdaddr, receive_arp_message->arp_srchaddr, 6); //mac�ּ�
			memcpy(arp_message.arp_destprotoaddr, receive_arp_message->arp_srcprotoaddr, 4); //ip�ּ�
			((CEthernetLayer*) this->mp_UnderLayer)->SetDestinAddress(receive_arp_message->arp_srchaddr, dev_num);
			routerDlg->m_EthernetLayer->Send((unsigned char *) &arp_message,ARP_MESSAGE_SIZE,arp_type,dev_num);
		}

		return TRUE;
	} else if(receive_arp_message->arp_op == reply){ //����
		if(!memcmp(receive_arp_message->arp_srcprotoaddr,routerDlg->m_IPLayer->GetSrcIP(dev_num),4)) //�ڽ��� ip = �߼��� ip�ǰ��
			AfxMessageBox("Ip�浹�Դϴ�.",0,0);

		else { //�ڽ��� ip != �߼��� ip
			//arp_message �ʱ�ȭ �� ����
			LPCACHE_ENTRY Cache_entry; //table �� insert�� cache
			Cache_entry = (LPCACHE_ENTRY)malloc(sizeof(CACHE_ENTRY));
			memcpy(Cache_entry->Ip_addr,receive_arp_message->arp_srcprotoaddr,4);
			memcpy(Cache_entry->Mac_addr,receive_arp_message->arp_srchaddr,6);
			Cache_entry->cache_ttl = 1200; //20��
			Cache_entry->cache_type = complete;

			if((index = SearchIpAtTable(Cache_entry->Ip_addr)) != -1) { //�����Ұ�� ������ȯ
				POSITION pos = Cache_Table.FindIndex(index);
				CACHE_ENTRY entry = Cache_Table.GetAt(pos);
				entry.cache_ttl = 1200;
				entry.cache_type = complete;
				memcpy(entry.Mac_addr, Cache_entry->Mac_addr, 6);
				Cache_Table.SetAt(pos, entry);
				updateCacheTable();
				free(Cache_entry); //�޸� ����
			} else //�������� ������� ���̺� ����
				InsertCache(Cache_entry); //cache insert

			// ���� Mac �ּҸ� ���� Buffer�� �����ߴ� data�� Send
			while(buf[out_index].valid == 1 && !memcmp(buf[out_index].dest_ip, receive_arp_message->arp_srcprotoaddr, 4)) { 
				//�������ڷ� send
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

int CARPLayer::SearchProxyTable(unsigned char Ip_addr[4]) 
{
	int count, i, ret = -1;
	PROXY_ENTRY temp;

	count = Proxy_Table.GetCount();
	for(i = 0; i < count; i++) {
		temp = Proxy_Table.GetAt(Proxy_Table.FindIndex(i));
		if(memcmp(Ip_addr ,temp.Ip_addr ,  4) == 0)
			ret = i; // index �� ����
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

BOOL CARPLayer::InsertProxy(CString name,unsigned char ip[4], unsigned char mac[6])
{
	PROXY_ENTRY entry;
	entry.Device_name.Format("%s",name);
	memcpy(entry.Ip_addr,ip,4);
	memcpy(entry.Mac_addr,mac,6);
	Proxy_Table.AddTail(entry);
	this->updateProxyTable();
	return TRUE;
}

BOOL CARPLayer::DeleteProxy(int index)
{
	//Proxy_Table.RemoveAt(index);
	return TRUE;
}

BOOL CARPLayer::DeleteAllProxy()
{
	//Proxy_Table.RemoveAll();
	return TRUE;
}

BOOL CARPLayer::ResetMessage()
{
	arp_message.arp_hdtype = htons(0x0001);
	arp_message.arp_prototype = htons(0x0800);
	arp_message.arp_hdlength = 0x06;
	arp_message.arp_protolength = 0x04;
	arp_message.arp_op = htons(0x0000); //2���� ����
	memset(arp_message.arp_srchaddr, 0, 6);
	memset(arp_message.arp_srcprotoaddr, 0, 4);
	memset(arp_message.arp_destprotoaddr, 0, 4);
	memset(arp_message.arp_desthdaddr, 0, 6);
	return TRUE;
}

BOOL CARPLayer::ResetMessageProxy()
{
	proxy_arp_message.arp_hdtype = htons(0x0001);
	proxy_arp_message.arp_prototype = htons(0x0800);
	proxy_arp_message.arp_hdlength = 0x06;
	proxy_arp_message.arp_protolength = 0x04;
	proxy_arp_message.arp_op = htons(0x0000); //2���� ����
	memset(proxy_arp_message.arp_srchaddr, 0, 6);
	memset(proxy_arp_message.arp_srcprotoaddr, 0, 4);
	memset(proxy_arp_message.arp_destprotoaddr, 0, 4);
	memset(proxy_arp_message.arp_desthdaddr, 0, 6);
	return TRUE;
}

//�ɽ� ���̺� ������Ʈ �Լ�
void CARPLayer::updateCacheTable()
{ 
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	routerDlg->ListBox_ARPCacheTable.DeleteAllItems(); //���� �ʱ�ȭ
	CString ip,mac,time,type;
	POSITION index;
	CACHE_ENTRY entry; //head position

	for(int i = 0; i < Cache_Table.GetCount(); i++) { //�ɽ� ���̺� ����������
		index = Cache_Table.FindIndex(i);
		entry = Cache_Table.GetAt(index);
		ip.Format("%d.%d.%d.%d", entry.Ip_addr[0], entry.Ip_addr[1], entry.Ip_addr[2], entry.Ip_addr[3]);
		mac.Format("%x-%x-%x-%x-%x-%x", entry.Mac_addr[0], entry.Mac_addr[1], entry.Mac_addr[2], entry.Mac_addr[3], entry.Mac_addr[4], entry.Mac_addr[5]);
		(entry.cache_type == complete ? type.Format("Complete") : type.Format("Incomplete"));
		time.Format("%d:%d",entry.cache_ttl/60, entry.cache_ttl%60);
		routerDlg->ListBox_ARPCacheTable.InsertItem(i,ip);
		routerDlg->ListBox_ARPCacheTable.SetItem(i,1,LVIF_TEXT,mac,0,0,0,NULL);
		routerDlg->ListBox_ARPCacheTable.SetItem(i,2,LVIF_TEXT,type,0,0,0,NULL);
		//routerDlg->ListBox_ARPCacheTable.SetItem(i,3,LVIF_TEXT,time,0,0,0,NULL);
	}

	routerDlg->ListBox_ARPCacheTable.UpdateWindow();
}

void CARPLayer::updateProxyTable()
{		
	CRouterDlg * routerDlg =  ((CRouterDlg *)(GetUnderLayer()->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)->GetUpperLayer(0)));
	routerDlg->ListBox_ARPProxyTable.DeleteAllItems();
	CString ip, mac, name;
	POSITION index1;
	PROXY_ENTRY entry1; //head position

	for(int i = 0; i < Proxy_Table.GetCount(); i++) {
		index1 = Proxy_Table.FindIndex(i);
		entry1 = Proxy_Table.GetAt(index1);
		ip.Format("%d.%d.%d.%d",entry1.Ip_addr[0],entry1.Ip_addr[1],entry1.Ip_addr[2],entry1.Ip_addr[3]);
		mac.Format("%x-%x-%x-%x-%x-%x",entry1.Mac_addr[0],entry1.Mac_addr[1],entry1.Mac_addr[2],entry1.Mac_addr[3], entry1.Mac_addr[4],entry1.Mac_addr[5]);
		name.Format("%s",entry1.Device_name);
		routerDlg->ListBox_ARPProxyTable.InsertItem(i,name);
		routerDlg->ListBox_ARPProxyTable.SetItem(i,1,LVIF_TEXT,ip,0,0,0,NULL);
		routerDlg->ListBox_ARPProxyTable.SetItem(i,2,LVIF_TEXT,mac,0,0,0,NULL);
	}

	routerDlg->ListBox_ARPProxyTable.UpdateWindow();
}

void CARPLayer::decreaseTime()
{
	if(!Cache_Table.IsEmpty()) {
		POSITION index;
		for(int i = 0; i < Cache_Table.GetCount(); i++) {
			index = Cache_Table.FindIndex(i);
			unsigned short ttl = Cache_Table.GetAt(index).cache_ttl -= 5;

			if(ttl <= 0) // ����
				Cache_Table.RemoveAt(index);
		}
		updateCacheTable();
	}
}

void CARPLayer::ResetCount()
{
	//sendCount = 0;
}
