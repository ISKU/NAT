#pragma once
#include "resource.h"
#include "RIPLayer.h"
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

protected:
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
	CRIPLayer		*m_RIPLayer;

	CLayerManager	 m_LayerMgr;
	pcap_if_t *Device1;
	pcap_if_t *Device2;
	CRoutTableAdder RtDlg;

public: 
	unsigned char *buf;
	pcap_if_t *Devices_1; //interface 0
	pcap_if_t *Devices_2; //interface 1

	typedef struct _RoutingTable {
		unsigned char ipAddress[4];
		unsigned char subnetmask[4];
		unsigned int metric;
		unsigned char out_interface;
		unsigned char nexthop[4];
		unsigned int status;
		unsigned int time;
	} RoutingTable, *RoutingTablePtr;

	static CList<RoutingTable, RoutingTable&> route_table;

public:
	CListCtrl ListBox_RoutingTable;
	CListCtrl ListBox_ARPCacheTable;
	CListCtrl ListBox_ICMPTable;
	afx_msg void OnBnClickedCacheDelete();
	afx_msg void OnBnClickedCacheDeleteAll();
	afx_msg void OnBnClickedProxyDelete();
	afx_msg void OnBnClickedProxyDeleteAll();
	afx_msg void OnBnClickedProxyAdd();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedNicSetButton();
	afx_msg void OnBnClickedRoutingAdd();
	afx_msg void OnBnClickedRoutingDelete();
	CComboBox m_nic1;
	CComboBox m_nic2;

	// NicList Set
	void setNicList(void);
	afx_msg void OnCbnSelchangeNic1Combo();
	//void add_route_table(unsigned char ipAddress[4], unsigned int metric, unsigned char dstInterface[4]);

	// UpdateRouteTable
	void UpdateRouteTable();
	afx_msg void OnCbnSelchangeNic2Combo();
	CIPAddressCtrl m_nic1_ip;
	CIPAddressCtrl m_nic2_ip;
	afx_msg void OnLvnItemchangedRoutingTable(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedRoutingTable2(NMHDR *pNMHDR, LRESULT *pResult);

	// RIP 응답 메시지 30초 Thread
	CWinThread* pThread_1;
	CWinThread* pThread_2;
	CWinThread* pThread_3;
	void StartReadThread();
	static unsigned int WaitRipResponseMessagePacket_1(LPVOID pParam);
	static unsigned int WaitRipResponseMessagePacket_2(LPVOID pParam);
	static unsigned int TableCheck(LPVOID pParam);
};
