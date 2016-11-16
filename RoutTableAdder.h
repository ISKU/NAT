#pragma once
#include "afxcmn.h"
#include "afxwin.h"

class CRoutTableAdder : public CDialog
{
	DECLARE_DYNAMIC(CRoutTableAdder)

public:
	CRoutTableAdder();   // ǥ�� �������Դϴ�.
	virtual ~CRoutTableAdder();
	enum { IDD = IDD_ROUTE_ADD_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	DECLARE_MESSAGE_MAP()

public:
	CString d1,d2;
	void setDeviceList(CString dev1,CString dev2);
	unsigned char table_ipAddress[4];
	unsigned int table_metric;
	unsigned char table_interface[4];

	// routetableDestination �߰�
	CIPAddressCtrl m_add_dest;
	CIPAddressCtrl m_add_netmask;
	CIPAddressCtrl m_gateway;
	CComboBox m_add_interface;

	CButton m_flag_u;
	CButton m_flag_g;
	CButton m_flag_h;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	unsigned char* GetIPAddress(void); // get ip
	unsigned int GetMetric(void); // get metric
	unsigned char* GetInterface(void); // get interface

	int m_metric;
};
