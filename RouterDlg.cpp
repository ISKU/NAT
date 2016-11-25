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

// 2개의 route_table (RouterDlg.h에 static으로 선언하여 route_table 멤버는 클래스 외부에 선언 본체는 Dialog가 가지고 있다)
CList<CRouterDlg::RoutingTable, CRouterDlg::RoutingTable&> CRouterDlg::route_table;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();
	enum { IDD = IDD_ABOUTBOX }; // 대화 상자 데이터입니다.

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

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

// CRouterDlg 대화 상자
CRouterDlg::CRouterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRouterDlg::IDD, pParent), CBaseLayer("CRouterDlg")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// Layer 생성
	m_NILayer = new CNILayer("NI");
	m_EthernetLayer = new CEthernetLayer("Ethernet");
	m_ARPLayer = new CARPLayer("ARP");
	m_IPLayer = new CIPLayer("IP");
	m_UDPLayer = new CUDPLayer("UDP"); // UDP Layer
	m_ICMPLayer = new CICMPLayer("ICMP"); // ICMP Layer
	m_TCPLayer = new CTCPLayer("TCP"); // TCP Layer

	// Layer 추가										
	m_LayerMgr.AddLayer( this );				
	m_LayerMgr.AddLayer( m_NILayer );			
	m_LayerMgr.AddLayer( m_EthernetLayer );
	m_LayerMgr.AddLayer( m_ARPLayer );
	m_LayerMgr.AddLayer( m_IPLayer );
	m_LayerMgr.AddLayer( m_UDPLayer ); // UDP Layer 추가
	m_LayerMgr.AddLayer( m_ICMPLayer ); // ICMP LAyer 추가
	m_LayerMgr.AddLayer( m_TCPLayer ); // TCP LAyer 추가

	// Layer연결 ///////////////////////////////////////////////////////////////////////////
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
	DDX_Control(pDX, IDC_ROUTING_TABLE, ListBox_RoutingTable);
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

// CRouterDlg 메시지 처리기
BOOL CRouterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.
	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	// 프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE); // 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE); // 작은 아이콘을 설정합니다.

	// ListBox에 초기 Colum을 삽입
	ListBox_RoutingTable.InsertColumn(0, _T(" "), LVCFMT_CENTER, 25, -1);
	ListBox_RoutingTable.InsertColumn(1,_T("Destination"),LVCFMT_CENTER, 115, -1);
	ListBox_RoutingTable.InsertColumn(2,_T("Metric"),LVCFMT_CENTER, 70, -1);
	ListBox_RoutingTable.InsertColumn(3,_T("Next Hop"),LVCFMT_CENTER, 115, -1);
	ListBox_RoutingTable.InsertColumn(4,_T("Interface"),LVCFMT_CENTER, 70, -1);
	ListBox_RoutingTable.InsertColumn(5,_T("Subnet Mask"),LVCFMT_CENTER, 115, -1);
	ListBox_RoutingTable.InsertColumn(6,_T("Status"),LVCFMT_CENTER, 50, -1);
	ListBox_RoutingTable.InsertColumn(7,_T("Time"),LVCFMT_CENTER, 50, -1);

	ListBox_ARPCacheTable.InsertColumn(0,_T("IP address"),LVCFMT_CENTER,100,-1);
	ListBox_ARPCacheTable.InsertColumn(1,_T("Mac address"),LVCFMT_CENTER,120,-1);
	ListBox_ARPCacheTable.InsertColumn(2,_T("Type"),LVCFMT_CENTER,80,-1);
	//ListBox_ARPCacheTable.InsertColumn(3,_T("Time"),LVCFMT_CENTER,49,-1);

	ListBox_ICMPTable.InsertColumn(0,_T("Inner address"),LVCFMT_CENTER,60,-1);
	ListBox_ICMPTable.InsertColumn(1,_T("Outer address"),LVCFMT_CENTER,120,-1);
	ListBox_ICMPTable.InsertColumn(2,_T("Identifier"),LVCFMT_CENTER,120,-1);
	ListBox_ICMPTable.InsertColumn(3,_T("Sequence"),LVCFMT_CENTER,120,-1);
	ListBox_ICMPTable.InsertColumn(4,_T("Time"),LVCFMT_CENTER,120,-1);

	setNicList(); //NicList Setting
	return TRUE; // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
// 아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
// 프레임워크에서 이 작업을 자동으로 수행합니다.
void CRouterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon); // 아이콘을 그립니다.
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서 이 함수를 호출합니다.
HCURSOR CRouterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRouterDlg::OnBnClickedCacheDelete()
{
	//CacheDeleteAll버튼
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
	//CacheDeleteAll버튼
	m_ARPLayer->Cache_Table.RemoveAll();
	m_ARPLayer->updateCacheTable();
}

void CRouterDlg::OnBnClickedCancel()
{
	exit(0); // 종료 버튼
}

void CRouterDlg::OnBnClickedNicSetButton()
{
	LPADAPTER adapter = NULL; // 랜카드에 대한 정보를 저장하는 pointer 변수
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
		printf("pcap_findalldevs_ex() error : %s\n", strError);\

	m_nic1.GetLBText(m_nic1.GetCurSel() , DeviceName1);	// 콤보 박스에 선택된 Device의 이름을 얻어옴
	m_nic2.GetLBText(m_nic2.GetCurSel() , DeviceName2);
	while(Devices != NULL) {
		if(!strcmp(Devices->description,DeviceName1))
			Device1 = Devices;
		if(!strcmp(Devices->description,DeviceName2))
			Device2 = Devices;
		Devices = Devices->next;
	}

	// device설정
	m_NILayer->SetDevice(Device1,1);
	m_NILayer->SetDevice(Device2,2);
	RtDlg.setDeviceList(Device1->description,Device2->description);

	//mac 주소 설정
	adapter = PacketOpenAdapter((Device1->name+8));
	PacketRequest( adapter, FALSE, OidData1);
	adapter = PacketOpenAdapter((Device2->name+8));
	PacketRequest( adapter, FALSE, OidData2);

	//ip주소 설정
	unsigned char nic1_ip[4];
	unsigned char nic2_ip[4];
	m_nic1_ip.GetAddress((BYTE &)nic1_ip[0],(BYTE &)nic1_ip[1],(BYTE &)nic1_ip[2],(BYTE &)nic1_ip[3]);
	m_nic2_ip.GetAddress((BYTE &)nic2_ip[0],(BYTE &)nic2_ip[1],(BYTE &)nic2_ip[2],(BYTE &)nic2_ip[3]);

	// 자기 자신의 라우팅 정보 업데이트
	unsigned char netmask[4] = { 0xff, 0xff, 0xff , 0 };
	
	RoutingTable rt1;
	for(int i=0; i<4; i++)
		rt1.ipAddress[i] = nic1_ip[i] & netmask[i];
	memcpy(rt1.subnetmask, netmask, 4);
	rt1.metric = 0x0;
	rt1.out_interface = 1;
	memset(&rt1.nexthop, 0, 4);
	rt1.status = 0;

	RoutingTable rt2;
	for(int i=0; i<4; i++)
		rt2.ipAddress[i] = nic2_ip[i] & netmask[i];
	memcpy(rt2.subnetmask, netmask, 4);
	rt2.metric = 0x0;
	rt2.out_interface = 2;
	memset(&rt2.nexthop, 0, 4);
	rt2.status = 0;

	route_table.AddTail(rt1);
	route_table.AddTail(rt2);
	UpdateRouteTable();
	////////////////////////////////////////////////////////////////////////

	m_NILayer->StartReadThread();	// receive Thread start
	GetDlgItem(IDC_NIC_SET_BUTTON)->EnableWindow(0);

	// 라우터 부팅시 request rip packet 전달
	unsigned char broadcast[4];
	memset(broadcast,0xff,4);
	unsigned char macbroadcast[6];
	memset(macbroadcast,0xff,6);

	m_EthernetLayer->SetDestinAddress(macbroadcast, 1);
	m_EthernetLayer->SetDestinAddress(macbroadcast, 2);
	m_IPLayer->SetDstIP(broadcast, 1);
	m_IPLayer->SetDstIP(broadcast, 2);
	m_IPLayer->SetSrcIP(nic1_ip, 1);
	m_IPLayer->SetSrcIP(nic2_ip, 2);
	m_EthernetLayer->SetSourceAddress(OidData1->Data,1);
	m_EthernetLayer->SetSourceAddress(OidData2->Data,2);

	//m_RIPLayer->Send(1, 1, 0);
	//m_RIPLayer->Send(1, 2, 0);
	//StartReadThread(); // RIP Response Thread start 30초
	/////////////////////////////////////////////////////////////////////
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
void CRouterDlg::UpdateRouteTable()
{
	RoutingTable entry;
	CString tableNumber, ipAddress, metric, out_interface, nexthop, subnetmask, status, time;
	int size = route_table.GetCount();

	// dev_num으로 구분하여 interface에 해당하는 route_table을 사용하여 CList에 있는 entry를 모두 레이아웃에 추가한다!
	ListBox_RoutingTable.DeleteAllItems();

	for(int index = 0; index < size; index++) {
		entry = route_table.GetAt(route_table.FindIndex(index));

		tableNumber.Format("%d", index + 1);
		ipAddress.Format("%d.%d.%d.%d", entry.ipAddress[0], entry.ipAddress[1], entry.ipAddress[2], entry.ipAddress[3]);
		metric.Format("%d", entry.metric);
		out_interface.Format("%d", entry.out_interface);
		nexthop.Format("%d.%d.%d.%d", entry.nexthop[0], entry.nexthop[1], entry.nexthop[2], entry.nexthop[3]);
		subnetmask.Format("%d.%d.%d.%d", entry.subnetmask[0], entry.subnetmask[1], entry.subnetmask[2], entry.subnetmask[3]);
		if (entry.status == 1) {
			status = "연결됨";
			time.Format("%d", entry.time);
		} else if (entry.status == 2) {
			status = "응답없음";
			time.Format("%d", entry.time);
		}
		else {
			status = "";
			time = "";
		}

		ListBox_RoutingTable.InsertItem(index, tableNumber);
		ListBox_RoutingTable.SetItem(index, 1, LVIF_TEXT, ipAddress, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 2, LVIF_TEXT, metric, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 3, LVIF_TEXT, nexthop, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 4, LVIF_TEXT, out_interface, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 5, LVIF_TEXT, subnetmask, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 6, LVIF_TEXT, status, 0, 0, 0, NULL);
		ListBox_RoutingTable.SetItem(index, 7, LVIF_TEXT, time, 0, 0, 0, NULL);
		
		ListBox_RoutingTable.UpdateWindow();
	}
}

void CRouterDlg::OnCbnSelchangeNic1Combo()
{
	// ip주소 설정
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
	//ip 주소 설정
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void CRouterDlg::OnLvnItemchangedRoutingTable2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

// 30초마다 RIP 응답 패킷을 보낸다
void CRouterDlg::StartReadThread()
{
	pThread_1 = AfxBeginThread(WaitRipResponseMessagePacket_1 , this);
	//pThread_2 = AfxBeginThread(WaitRipResponseMessagePacket_2 , this);
	pThread_3 = AfxBeginThread(TableCheck , this);

	if(pThread_1 == NULL || pThread_3 == NULL) {
		AfxMessageBox("Read 쓰레드 생성 실패");
	}
	/*
	if(pThread_1 == NULL || pThread_2 == NULL || pThread_3 == NULL) {
		AfxMessageBox("Read 쓰레드 생성 실패");
	}
	*/
}

unsigned int CRouterDlg::WaitRipResponseMessagePacket_1(LPVOID pParam) 
{
	CRouterDlg *temp_CRouterDlgLayer = (CRouterDlg*)pParam;

	while(1) {
		Sleep(7000);
		temp_CRouterDlgLayer->GetUnderLayer()->Send(2, 1, 0);
		temp_CRouterDlgLayer->GetUnderLayer()->Send(2, 2, 0);
	}

	return 0;
}

unsigned int CRouterDlg::WaitRipResponseMessagePacket_2(LPVOID pParam){
	CRouterDlg *temp_CRouterDlgLayer = (CRouterDlg*)pParam;

	while(1) {
		Sleep(7000);
		temp_CRouterDlgLayer->GetUnderLayer()->Send(2, 2, 0);
	}

	return 0;
}

unsigned int CRouterDlg::TableCheck(LPVOID pParam){
	CList<RoutingTable, RoutingTable&> *temp_route_table;
	RoutingTable entry;

	while(1) {
		temp_route_table = &(((CRouterDlg*)pParam)->route_table);
		for (int index = 0; index < temp_route_table->GetCount(); index++) {
			entry = temp_route_table->GetAt(temp_route_table->FindIndex(index));
			if (entry.status == 1) {
				if (entry.time != 0)
					entry.time = entry.time - 1;
				else {
					entry.status = 2;
					entry.metric = 16;
					entry.time = 5;
				}
				temp_route_table->SetAt(temp_route_table->FindIndex(index), entry);
			} else if (entry.status == 2) {
				if(entry.time != 0) {
					entry.time = entry.time - 1;
					temp_route_table->SetAt(temp_route_table->FindIndex(index), entry);
				} else {
					temp_route_table->RemoveAt(temp_route_table->FindIndex(index));
					index--;
				}
			}
		}
		((CRouterDlg*)pParam)->UpdateRouteTable();
		Sleep(10000);
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////

void CRouterDlg::OnLvnItemchangedIcmpTable(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}
