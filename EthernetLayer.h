#pragma once
#include "baselayer.h"

class CEthernetLayer : public CBaseLayer  
{
private:
	inline void	ResetHeader(); // header를 초기화 하는 함수

public:
	BOOL Receive(unsigned char* ppayload,int dev_num);		
	BOOL Send(unsigned char* ppayload, int nlength, int dev_num);

	//mac1
	BOOL			Send( unsigned char* ppayload, int nlength ,unsigned short type, int dev_num);
	void			SetDestinAddress(unsigned char* pAddress, int dev_num);
	void			SetSourceAddress(unsigned char* pAddress, int dev_num);
	void			SetSourceAddressForRip(unsigned char* pAddress, int dev_num);
	unsigned char*	GetDestinAddress();
	unsigned char*	GetSourceAddressForRip(int dev_num);
	unsigned char*	GetSourceAddress(int dev_num);

	//mac2
	void SetType(unsigned short type);
	CEthernetLayer(char* pName);
	virtual ~CEthernetLayer();

	// Ethernet의 주소를 저장하는 구조체 union으로 선언되어 있다.
	typedef struct _ETHERNET_ADDR {
		union {
			struct {unsigned char e0, e1, e2, e3, e4, e5;} s_un_byte;
			unsigned char s_ether_addr[6];
		} S_un;

		#define addr0 S_un.s_un_byte.e0
		#define addr1 S_un.s_un_byte.e1
		#define addr2 S_un.s_un_byte.e2
		#define addr3 S_un.s_un_byte.e3
		#define addr4 S_un.s_un_byte.e4
		#define addr5 S_un.s_un_byte.e5
		#define addr_ethernet S_un.s_ether_addr
	} ETHERNET_ADDR, *LPETHERNET_ADDR; // 주소에 대한 구조체 임.
	
	//실제 Ethernet의 header 부분 
	typedef struct _ETHERNET {
		ETHERNET_ADDR Ethernet_dstAddr;	// 상대방 주소 - mac 주소 , 6
		ETHERNET_ADDR Ethernet_srcAddr;	// 자기 주소 , 6
		unsigned short Ethernet_type; // frame data type , 2
		unsigned char Ethernet_data[ETHERNET_MAX_DATA]; // data 부분은 46-1500가 되어야 하고 최대는 1500이다. 
	} EthernetHeader, *PEthernetHeader;	// header부분의 크기는 6+6+2 = 14 이다.
	
	unsigned char dev_1_mac_addr[6];
	unsigned char dev_2_mac_addr[6];
	unsigned char dev_1_dst_mac_addr[6];
	unsigned char dev_2_dst_mac_addr[6];
	unsigned char dev_1_mac_addr_for_rip[6];
	unsigned char dev_2_mac_addr_for_rip[6];
	
	//protected:
	EthernetHeader	Ethernet_Header; // Ethernet header에 대한 구조체 변수를 선언해 놓는다.
};