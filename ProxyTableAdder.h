#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "Router.h"

// ProxyTableAdder dialog
class ProxyTableAdder : public CDialog
{
	DECLARE_DYNAMIC(ProxyTableAdder)

public:
	ProxyTableAdder(CWnd* pParent = NULL); // standard constructor
	virtual ~ProxyTableAdder();

	CString name;
	unsigned char ip[4];
	unsigned char mac[6];
	char Buffer[16];
	char Buffer1[18];

	// Dialog Data
	enum { IDD = IDD_PROXY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_Nic_Name;
	CIPAddressCtrl m_ip;
	CListBox m_mac;
	unsigned char* getIp() { return ip; }
	unsigned char* getMac() { return mac; }
	CString getName() { return name; }

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
