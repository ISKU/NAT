#pragma once
#include "resource.h"
#include "TCPLayer.h"
#include "ICMPLayer.h"
#include "UDPLayer.h"
#include "ARPLayer.h"
#include "IPLayer.h"
#include "NILayer.h"
#include "EthernetLayer.h"
#include "LayerManager.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "RoutTableAdder.h"

// CRouterDlg 대화 상자
class CRouterDlg : public CDialog, public CBaseLayer
{
	// 생성입니다.
public:
	CRouterDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	enum { IDD = IDD_STATICROUTER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public: //layer
	CNILayer		*m_NILayer;
	CEthernetLayer	*m_EthernetLayer;
	CARPLayer		*m_ARPLayer;
	CIPLayer		*m_IPLayer;
	CUDPLayer		*m_UDPLayer;
	CICMPLayer		*m_ICMPLayer;
	CTCPLayer		*m_TCPLayer;

	CLayerManager	 m_LayerMgr;
	pcap_if_t *Device1;
	pcap_if_t *Device2;
	CRoutTableAdder RtDlg;

	unsigned char *buf;
	pcap_if_t *Devices_1; //interface 0
	pcap_if_t *Devices_2; //interface 1

	typedef struct _TRANSPORT_ENTRY {
		unsigned char inner_addr[4];
		unsigned short inner_port;
		unsigned short outer_port;
		unsigned int status;
		unsigned int time;
	} NAT_ENTRY, *PNAT_ENTRY;

	static CList<NAT_ENTRY, NAT_ENTRY&> nat_table;

	CListCtrl ListBox_NatTable;
	CListCtrl ListBox_ARPCacheTable;
	CListCtrl ListBox_ICMPTable;
	afx_msg void OnBnClickedCacheDelete();
	afx_msg void OnBnClickedCacheDeleteAll();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedNicSetButton();
	afx_msg void OnBnClickedRoutingAdd();
	afx_msg void OnBnClickedRoutingDelete();
	void setNicList(void);
	void UpdateNatTable();
	CComboBox m_nic1;
	CComboBox m_nic2;

	/*내가추가*/
	char * Devices1_ip;
	char * Devices2_ip;

	// NicList Set
	afx_msg void OnCbnSelchangeNic1Combo();
	afx_msg void OnCbnSelchangeNic2Combo();
	CIPAddressCtrl m_nic1_ip;
	CIPAddressCtrl m_nic2_ip;
	afx_msg void OnLvnItemchangedRoutingTable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedRoutingTable2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedIcmpTable(NMHDR *pNMHDR, LRESULT *pResult);

	CWinThread* pThread_1;
	void StartReadThread();
	static unsigned int TableCheck(LPVOID pParam);

	unsigned char* GetSrcIP(int dev_num);
	unsigned char* GetSrcMAC(int dev_num);

	unsigned char public_IP[4];
	unsigned char private_IP[4];
	unsigned char public_MAC[6];
	unsigned char private_MAC[6];
};
