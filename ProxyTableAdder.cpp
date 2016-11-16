#include "stdafx.h"
#include "Router.h"
#include "ProxyTableAdder.h"
#include "RouterDlg.h"

// ProxyTableAdder dialog
IMPLEMENT_DYNAMIC(ProxyTableAdder, CDialog)

	ProxyTableAdder::ProxyTableAdder(CWnd* pParent /*=NULL*/) : CDialog(ProxyTableAdder::IDD, pParent)
{
}

ProxyTableAdder::~ProxyTableAdder()
{
}

void ProxyTableAdder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_Nic_Name);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ip);
	DDX_Control(pDX, IDC_LIST3, m_mac);
}

BEGIN_MESSAGE_MAP(ProxyTableAdder, CDialog)
	ON_BN_CLICKED(IDOK, &ProxyTableAdder::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &ProxyTableAdder::OnBnClickedCancel)
END_MESSAGE_MAP()

// ProxyTableAdder message handlers
void ProxyTableAdder::OnBnClickedOk()
{
	m_Nic_Name.GetLBText(0 , name);
	m_ip.GetWindowText(Buffer , sizeof(Buffer));
	sscanf(Buffer, "%d.%d.%d.%d",&ip[0] , &ip[1] , &ip[2] , &ip[3] , sizeof(Buffer));

	m_mac.GetWindowText(Buffer1 , sizeof(Buffer1));
	sscanf(Buffer1, "%x-%x-%x-%x-%x-%x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5], sizeof(Buffer1));

	CDialog::OnOK();
	OnOK();
}

void ProxyTableAdder::OnBnClickedCancel()
{
	OnCancel();
}