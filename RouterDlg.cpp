#pragma comment( lib, "iphlpapi.lib" )
#include "stdafx.h"
#include "Router.h"
#include "RouterDlg.h"
#include "RoutTableAdder.h"
#include "IPLayer.h"

#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 2���� route_table (RouterDlg.h�� static���� �����Ͽ� route_table ����� Ŭ���� �ܺο� ���� ��ü�� Dialog�� ������ �ִ�)
CList<CRouterDlg::NAT_ENTRY, CRouterDlg::NAT_ENTRY&> CRouterDlg::nat_table;

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX }; // ��ȭ ���� �������Դϴ�.

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV �����Դϴ�.

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// CRouterDlg ��ȭ ����
CRouterDlg::CRouterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRouterDlg::IDD, pParent), CBaseLayer("CRouterDlg")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// Layer ����
	m_NILayer = new CNILayer("NI");
	m_EthernetLayer = new CEthernetLayer("Ethernet");
	m_ARPLayer = new CARPLayer("ARP");
	m_IPLayer = new CIPLayer("IP");
	m_UDPLayer = new CUDPLayer("UDP"); // UDP Layer
	m_ICMPLayer = new CICMPLayer("ICMP"); // ICMP Layer
	m_TCPLayer = new CTCPLayer("TCP"); // TCP Layer

	// Layer �߰�										
	m_LayerMgr.AddLayer( this );				
	m_LayerMgr.AddLayer( m_NILayer );			
	m_LayerMgr.AddLayer( m_EthernetLayer );
	m_LayerMgr.AddLayer( m_ARPLayer );
	m_LayerMgr.AddLayer( m_IPLayer );
	m_LayerMgr.AddLayer( m_UDPLayer ); // UDP Layer �߰�
	m_LayerMgr.AddLayer( m_ICMPLayer ); // ICMP LAyer �߰�
	m_LayerMgr.AddLayer( m_TCPLayer ); // TCP LAyer �߰�

	// Layer���� ///////////////////////////////////////////////////////////////////////////
	m_NILayer->SetUpperLayer(m_EthernetLayer);

	m_EthernetLayer->SetUpperLayer(m_IPLayer);
	m_EthernetLayer->SetUpperLayer(m_ARPLayer);
	m_EthernetLayer->SetUnderLayer(m_NILayer);

	m_ARPLayer->SetUnderLayer(m_EthernetLayer);

	m_IPLayer->SetUpperLayer(m_ICMPLayer);
	m_IPLayer->SetUpperLayer(m_TCPLayer);
	m_IPLayer->SetUpperLayer(m_UDPLayer);
	m_IPLayer->SetUnderLayer(m_ARPLayer);

	m_ICMPLayer->SetUpperLayer(this);
	m_ICMPLayer->SetUnderLayer(m_IPLayer);

	m_UDPLayer->SetUpperLayer(this);
	m_UDPLayer->SetUnderLayer(m_IPLayer);

	m_TCPLayer->SetUpperLayer(this);
	m_TCPLayer->SetUnderLayer(m_IPLayer);

	this->SetUnderLayer(m_ICMPLayer);
	this->SetUnderLayer(m_UDPLayer);
	this->SetUnderLayer(m_TCPLayer);
	/////////////////////////////////////////////////////////////////////////////////////////
}

void CRouterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ROUTING_TABLE, ListBox_NatTable);
	DDX_Control(pDX, IDC_CACHE_TABLE, ListBox_ARPCacheTable);
	DDX_Control(pDX, IDC_ICMP_TABLE, ListBox_ICMPTable);
	DDX_Control(pDX, IDC_NIC1_COMBO, m_nic1);
	DDX_Control(pDX, IDC_NIC2_COMBO, m_nic2);
	DDX_Control(pDX, IDC_IPADDRESS1, m_nic1_ip);
	DDX_Control(pDX, IDC_IPADDRESS2, m_nic2_ip);
}

BEGIN_MESSAGE_MAP(CRouterDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CACHE_DELETE, &CRouterDlg::OnBnClickedCacheDelete)
	ON_BN_CLICKED(IDC_CACHE_DELETE_ALL, &CRouterDlg::OnBnClickedCacheDeleteAll)
	ON_BN_CLICKED(IDCANCEL, &CRouterDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_NIC_SET_BUTTON, &CRouterDlg::OnBnClickedNicSetButton)
	ON_CBN_SELCHANGE(IDC_NIC1_COMBO, &CRouterDlg::OnCbnSelchangeNic1Combo)
	ON_CBN_SELCHANGE(IDC_NIC2_COMBO, &CRouterDlg::OnCbnSelchangeNic2Combo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ROUTING_TABLE, &CRouterDlg::OnLvnItemchangedRoutingTable)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ROUTING_TABLE2, &CRouterDlg::OnLvnItemchangedRoutingTable2)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ICMP_TABLE, &CRouterDlg::OnLvnItemchangedIcmpTable)
END_MESSAGE_MAP()

// CRouterDlg �޽��� ó����
BOOL CRouterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.
	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	// �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE); // ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE); // ���� �������� �����մϴ�.

	// ListBox�� �ʱ� Colum�� ����
	ListBox_NatTable.InsertColumn(0, _T(""), LVCFMT_CENTER, 0, -1);
	ListBox_NatTable.InsertColumn(1,_T("Inner address"),LVCFMT_CENTER, 150, -1);
	ListBox_NatTable.InsertColumn(2,_T("Inner port"),LVCFMT_CENTER, 110, -1);
	ListBox_NatTable.InsertColumn(3,_T("Outer port"),LVCFMT_CENTER, 150, -1);
	ListBox_NatTable.InsertColumn(4,_T("Status"),LVCFMT_CENTER, 100, -1);
	ListBox_NatTable.InsertColumn(5,_T("Time"),LVCFMT_CENTER, 103, -1);

	ListBox_ARPCacheTable.InsertColumn(0,_T(""),LVCFMT_CENTER,0,-1);
	ListBox_ARPCacheTable.InsertColumn(1,_T("IP address"),LVCFMT_CENTER,180,-1);
	ListBox_ARPCacheTable.InsertColumn(2,_T("Mac address"),LVCFMT_CENTER,180,-1);
	ListBox_ARPCacheTable.InsertColumn(3,_T("Type"),LVCFMT_CENTER,102,-1);
	//ListBox_ARPCacheTable.InsertColumn(3,_T("Time"),LVCFMT_CENTER,49,-1);

	ListBox_ICMPTable.InsertColumn(0,_T(""),LVCFMT_CENTER,0,-1);
	ListBox_ICMPTable.InsertColumn(1,_T("Inner address"),LVCFMT_CENTER,120,-1);
	ListBox_ICMPTable.InsertColumn(2,_T("Outer address"),LVCFMT_CENTER,120,-1);
	ListBox_ICMPTable.InsertColumn(3,_T("Identifier"),LVCFMT_CENTER,80,-1);
	ListBox_ICMPTable.InsertColumn(4,_T("Seq #"),LVCFMT_CENTER,80,-1);
	ListBox_ICMPTable.InsertColumn(5,_T("Time"),LVCFMT_CENTER,60,-1);

	setNicList(); //NicList Setting
	return TRUE; // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CRouterDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
// �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
// �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.
void CRouterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon); // �������� �׸��ϴ�.
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ��� �� �Լ��� ȣ���մϴ�.
HCURSOR CRouterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRouterDlg::OnBnClickedCacheDelete()
{
	//CacheDeleteAll��ư
	int index = -1;

	index = ListBox_ARPCacheTable.GetSelectionMark();
	if(index != -1){
		POSITION pos = m_ARPLayer->Cache_Table.FindIndex(index);
		m_ARPLayer->Cache_Table.RemoveAt(pos);
		m_ARPLayer->updateCacheTable();
	}
}

void CRouterDlg::OnBnClickedCacheDeleteAll()
{
	//CacheDeleteAll��ư
	m_ARPLayer->Cache_Table.RemoveAll();
	m_ARPLayer->updateCacheTable();
}

void CRouterDlg::OnBnClickedCancel()
{
	exit(0); // ���� ��ư
}

void CRouterDlg::OnBnClickedNicSetButton()
{
	LPADAPTER adapter = NULL; // ��ī�忡 ���� ������ �����ϴ� pointer ����
	PPACKET_OID_DATA OidData1;
	PPACKET_OID_DATA OidData2;
	pcap_if_t *Devices;

	OidData1 = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData1->Oid = 0x01010101;
	OidData1->Length = 6;

	OidData2 = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData2->Oid = 0x01010101;
	OidData2->Length = 6;


	ZeroMemory(OidData1->Data,6);
	ZeroMemory(OidData2->Data,6);
	char DeviceName1[512];
	char DeviceName2[512];
	char strError[30];

	if(pcap_findalldevs_ex( PCAP_SRC_IF_STRING, NULL , &Devices , strError) != 0)
		printf("pcap_findalldevs_ex() error : %s\n", strError);

	m_nic1.GetLBText(m_nic1.GetCurSel() , DeviceName1);	// �޺� �ڽ��� ���õ� Device�� �̸��� ����
	m_nic2.GetLBText(m_nic2.GetCurSel() , DeviceName2);
	while(Devices != NULL) {
		if(!strcmp(Devices->description,DeviceName1))
			Device1 = Devices;
		if(!strcmp(Devices->description,DeviceName2))
			Device2 = Devices;
		Devices = Devices->next;
	}

	// device����
	m_NILayer->SetDevice(Device1,1);
	m_NILayer->SetDevice(Device2,2);
	RtDlg.setDeviceList(Device1->description,Device2->description);

	//mac �ּ� ����
	adapter = PacketOpenAdapter((Device1->name+8));
	PacketRequest( adapter, FALSE, OidData1);
	adapter = PacketOpenAdapter((Device2->name+8));
	PacketRequest( adapter, FALSE, OidData2);

	//ip�ּ� ����
	unsigned char nic1_ip[4];
	unsigned char nic2_ip[4];
	m_nic1_ip.GetAddress((BYTE &)nic1_ip[0],(BYTE &)nic1_ip[1],(BYTE &)nic1_ip[2],(BYTE &)nic1_ip[3]);
	m_nic2_ip.GetAddress((BYTE &)nic2_ip[0],(BYTE &)nic2_ip[1],(BYTE &)nic2_ip[2],(BYTE &)nic2_ip[3]);

	memcpy(public_IP, nic1_ip, 4);
	memcpy(private_IP, nic2_ip, 4);
	memcpy(public_MAC, OidData1->Data, 6);
	memcpy(private_MAC, OidData2->Data, 6);

	circularIndex = 0;
	m_NILayer->StartReadThread();	// receive Thread start
	m_ICMPLayer->StartReadThread();
	StartReadThread(); // Router Table Thread Start
	GetDlgItem(IDC_NIC_SET_BUTTON)->EnableWindow(0);
}

// NicList Set
void CRouterDlg::setNicList(void)
{
	pcap_if_t *Devices;
	char strError[30];

	if(pcap_findalldevs_ex( PCAP_SRC_IF_STRING, NULL , &Devices , strError) != 0)
		printf("pcap_findalldevs_ex() error : %s\n", strError);

	// set device
	while(Devices != NULL) {
		m_nic1.AddString(Devices->description);
		m_nic2.AddString(Devices->description);
		Devices = Devices->next;
	}

	m_nic1.SetCurSel(0);
	m_nic2.SetCurSel(1);
}	

// UpdateRouteTable
void CRouterDlg::UpdateNatTable()
{
	NAT_ENTRY entry;
	CString innerAddrees, innerPort, outerPort, status, time;

	// dev_num���� �����Ͽ� interface�� �ش��ϴ� route_table�� ����Ͽ� CList�� �ִ� entry�� ��� ���̾ƿ��� �߰��Ѵ�!
	ListBox_NatTable.DeleteAllItems();

	for(int index = 0; index < nat_table.GetCount(); index++) {
		entry = nat_table.GetAt(nat_table.FindIndex(index));

		innerAddrees.Format("%d.%d.%d.%d", entry.inner_addr[0], entry.inner_addr[1], entry.inner_addr[2], entry.inner_addr[3]);
		innerPort.Format("%d", entry.inner_port);
		outerPort.Format("%d", entry.outer_port);

		if (entry.status == 10) {
			status = "UDP";
		} else {
			status = "TCP";
		}
		time.Format("%d", entry.time);

		ListBox_NatTable.InsertItem(index, NULL);
		ListBox_NatTable.SetItem(index, 1, LVIF_TEXT, innerAddrees, 0, 0, 0, NULL);
		ListBox_NatTable.SetItem(index, 2, LVIF_TEXT, innerPort, 0, 0, 0, NULL);
		ListBox_NatTable.SetItem(index, 3, LVIF_TEXT, outerPort, 0, 0, 0, NULL);
		ListBox_NatTable.SetItem(index, 4, LVIF_TEXT, status, 0, 0, 0, NULL);
		ListBox_NatTable.SetItem(index, 5, LVIF_TEXT, time, 0, 0, 0, NULL);
		
		ListBox_NatTable.UpdateWindow();
	}
}

void CRouterDlg::OnCbnSelchangeNic1Combo()
{
	// ip�ּ� ����
	char DeviceName1[512];
	m_nic1.GetLBText(m_nic1.GetCurSel() , DeviceName1);
    int a_devnum=m_nic1.GetCurSel();
	ULONG    ulBufferSize = 0;
	PIP_ADAPTER_INFO pAdapter, pAdapterInfo = NULL;

	if (GetAdaptersInfo(NULL, &ulBufferSize) == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[ulBufferSize];
		if (pAdapterInfo)
		{
			if (GetAdaptersInfo(pAdapterInfo, &ulBufferSize) == ERROR_SUCCESS)
			{
				pAdapter = pAdapterInfo;
				for(int FindDeviceCount = 0 ; FindDeviceCount < a_devnum; FindDeviceCount++){
					pAdapterInfo = pAdapterInfo->Next;
				}
				Devices1_ip =  pAdapterInfo->IpAddressList.IpAddress.String;
			}
		}
	}

	/*change char to byte */
	char addr[4][5];
    int k=0;
    memset(addr, 0, 20);
       
    for(int i=0 ; i<4 ;i++)
    {     
		for(int j=0 ; 1 ; j++, k++)
        {       
			if( (Devices1_ip[k] == '.') || (Devices1_ip[k] == NULL) )
            {      addr[i][j] = '\0';
                   k++;
                   break;
            }
			addr[i][j] = Devices1_ip[k];
        }
    }

	BYTE b[4] = {atoi(addr[0]), atoi(addr[1]), atoi(addr[2]), atoi(addr[3]) };
	m_nic1_ip.SetAddress(b[0], b[1], b[2], b[3]);
   
}


void CRouterDlg::OnCbnSelchangeNic2Combo()
{
	//ip �ּ� ����
	char DeviceName2[512];
	m_nic2.GetLBText(m_nic2.GetCurSel() , DeviceName2);
    int a_devnum=m_nic2.GetCurSel();
	ULONG    ulBufferSize = 0;
	PIP_ADAPTER_INFO pAdapter, pAdapterInfo = NULL;

	if (GetAdaptersInfo(NULL, &ulBufferSize) == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[ulBufferSize];
		if (pAdapterInfo)
		{
			if (GetAdaptersInfo(pAdapterInfo, &ulBufferSize) == ERROR_SUCCESS)
			{
				pAdapter = pAdapterInfo;
				for(int FindDeviceCount = 0 ; FindDeviceCount < a_devnum; FindDeviceCount++){
					pAdapterInfo = pAdapterInfo->Next;
				}
				Devices2_ip =  pAdapterInfo->IpAddressList.IpAddress.String;
			}
		}
	}

	/*change char to byte */
	char addr[4][5];
    int k=0;
    memset(addr, 0, 20);
       
    for(int i=0 ; i<4 ;i++)
    {     
		for(int j=0 ; 1 ; j++, k++)
        {       
			if( (Devices2_ip[k] == '.') || (Devices2_ip[k] == NULL) )
            {      addr[i][j] = '\0';
                   k++;
                   break;
            }
			addr[i][j] = Devices2_ip[k];
        }
    }

	BYTE b[4] = {atoi(addr[0]), atoi(addr[1]), atoi(addr[2]), atoi(addr[3]) };
	m_nic2_ip.SetAddress(b[0], b[1], b[2], b[3]);
}

void CRouterDlg::OnLvnItemchangedRoutingTable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
}


void CRouterDlg::OnLvnItemchangedRoutingTable2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
}

void CRouterDlg::OnLvnItemchangedIcmpTable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	*pResult = 0;
}

void CRouterDlg::StartReadThread()
{
	pThread_1 = AfxBeginThread(TableCheck , this);

	if(pThread_1 == NULL)
		AfxMessageBox("Read ������ ���� ����");
}


unsigned int CRouterDlg::TableCheck(LPVOID pParam){
	NAT_ENTRY entry;

	while(1) {
		for (int index = 0; index < nat_table.GetCount(); index++) {
			entry = nat_table.GetAt(nat_table.FindIndex(index));

			if (entry.time != 0) {
				entry.time = entry.time - 1;
				nat_table.SetAt(nat_table.FindIndex(index), entry);
			} else {
				nat_table.RemoveAt(nat_table.FindIndex(index));
				index--;
			}
		}

		((CRouterDlg*)pParam)->UpdateNatTable();
		Sleep(1500);
	}
	
	return 0;
}

unsigned char* CRouterDlg::GetSrcIP(int dev_num)
{
	if (dev_num == DEV_PUBLIC)
		return public_IP;
	return private_IP;
}

unsigned char* CRouterDlg::GetSrcMAC(int dev_num)
{
	if (dev_num == DEV_PUBLIC)
		return public_MAC;
	return private_MAC;
}

int CRouterDlg::SearchOutgoingTable(unsigned char inner_addr[4], unsigned short inner_port) {
	CRouterDlg::NAT_ENTRY entry;

	for(int index = 0; index < CRouterDlg::nat_table.GetCount(); index++) {
		entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
		if (!memcmp(entry.inner_addr, inner_addr, 4) && entry.inner_port == inner_port) 
			return index;
	}
	return -1;
}

int CRouterDlg::SearchIncomingTable(unsigned short outer_port) {
	CRouterDlg::NAT_ENTRY entry;

	for(int index = 0; index < CRouterDlg::nat_table.GetCount(); index++) {
		entry = CRouterDlg::nat_table.GetAt(CRouterDlg::nat_table.FindIndex(index));
		if (entry.outer_port == outer_port) 
			return index;
	}
	return -1;
}

int CRouterDlg::GetCircularIndex() {
	int index = circularIndex + 49152;
	circularIndex = (circularIndex + 1) % 16383;

	return index;
}