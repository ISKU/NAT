#pragma once
#include "baselayer.h"
//#include "RouterDlg.h"

class CARPLayer : public CBaseLayer
{
public:
	typedef struct _CACHE_ENTRY {
		unsigned char Ip_addr[4];
		unsigned char Mac_addr[6];
		unsigned char cache_type; //type;
		unsigned short cache_ttl; //time
	} CACHE_ENTRY, *LPCACHE_ENTRY;

	// -- Proxy Table struct
	typedef struct _PROXY_ENTRY {
		unsigned char Ip_addr[4]; //ip
		unsigned char Mac_addr[6]; //mac
		CString Device_name;
	} PROXY_ENTRY, *LPPROXY_ENTRY;

	// -- Arp Message strcut
	typedef struct _ARP_Message {
		unsigned short arp_hdtype;
		unsigned short arp_prototype;
		unsigned char arp_hdlength;
		unsigned char arp_protolength;
		unsigned short arp_op;

		unsigned char arp_srchaddr[6]; //src_mac
		unsigned char arp_srcprotoaddr[4]; //src_ip
		unsigned char arp_desthdaddr[6]; //dest_mac
		unsigned char arp_destprotoaddr[4]; //src_ip;
	} ARP_Message, *LPARP_Message;

	CARPLayer(char *pName);
	~CARPLayer(void);

	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);
	BOOL Receive(unsigned char* ppayload, int dev_num);
	int SearchIpAtTable(unsigned char Ip_addr[4]); //ip찾는 함수
	int SearchProxyTable(unsigned char IP_addr[4]); //proxytable에서 찾는함수

	void updateCacheTable();
	void updateProxyTable();

	BOOL InsertCache(LPCACHE_ENTRY Cache_entry);
	BOOL DeleteCache(int index);
	BOOL DeleteAllCache();

	BOOL InsertProxy(CString name,unsigned char ip[4],unsigned char mac[6]);
	BOOL DeleteProxy(int index);
	BOOL DeleteAllProxy();

	BOOL ResetMessage();
	BOOL ResetMessageProxy();
	void decreaseTime();

	CList<CACHE_ENTRY,CACHE_ENTRY&> Cache_Table;
	CList<PROXY_ENTRY,PROXY_ENTRY&> Proxy_Table;

	BOOL Receive() {return FALSE;}
	void ResetCount();

private:
	typedef struct _buffer {
		unsigned char *data;
		int valid;
		int dev_num;
		int nlength;
		unsigned char dest_ip[4];
	} arp_buffer;
	arp_buffer buf[2]; //버퍼로 사용

	int buf_index;
	int out_index;
	int sendCount; //send 횟수
	ARP_Message arp_message; //arp_message
	ARP_Message proxy_arp_message; //proxy_arp_message
};
