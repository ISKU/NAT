// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS
#define HAVE_REMOTE
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <pcap.h>			// 추가한 부분
//#include <remote-ext.h>	// 추가한 부분
#include "Packet32.h"		// 추가한 부분
#include <String.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h> // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h> // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#define MAX_LAYER_NUMBER		0xff	

#define ETHERNET_MAX_SIZE			1514
#define ETHERNET_HEADER_SIZE		14
#define ETHERNET_MAX_DATA			( ETHERNET_MAX_SIZE - ETHERNET_HEADER_SIZE )

#define IP_HEADER_SIZE				20
#define IP_MAX_DATA					( ETHERNET_MAX_DATA - IP_HEADER_SIZE )

#define TCP_HEADER_SIZE				20
#define TCP_MAX_DATA				( IP_MAX_DATA - TCP_HEADER_SIZE )

#define UDP_HEADER_SIZE				8
#define UDP_MAX_DATA				( IP_MAX_DATA - UDP_HEADER_SIZE )

#define TCP_HEADER_SIZE				20
#define TCP_MAX_DATA				( IP_MAX_DATA - TCP_HEADER_SIZE )

#define ICMP_HEADER_SIZE			8
#define ICMP_MAX_DATA				32

#define UDP_PSEUDO_HEADER_SIZE		12

#define RIP_HEADER_SIZE				4
#define RIP_MAX_DATA				( UDP_MAX_DATA - RIP_HEADER_SIZE )

#define DEV_PUBLIC					1
#define DEV_PRIVATE					2

#define complete					0x00
#define incomplete					0x01

#define request						htons(0x01)
#define reply						htons(0x02)

#define ARP_MESSAGE_SIZE			28

#define delete_timer				0x01 // 1초마다 삭제 타이머
#define wait_timer					0x02 // 2초 기다리는 타이머
#define gratuitous_timer			0x03 //gratuitous timer

#define ip_type						htons(0x0800)
#define arp_type					htons(0x0806)

#define INFINITY_HOP_COUNT			16

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


