#if !defined(AFX_BASELAYER_H__041C5A07_23A9_4CBC_970B_8743460A7DA9__INCLUDED_)
#define AFX_BASELAYER_H__041C5A07_23A9_4CBC_970B_8743460A7DA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

class CBaseLayer  
{
public:
	char* GetLayerName(); // 계층의 이름을 얻어오는 함수
	CBaseLayer* GetUnderLayer(); // 하위 계층을 얻어오는 함수
	CBaseLayer* GetUpperLayer(int nindex); // nindex에 해당 하는 상위 계층을 얻어오는 함수

	void SetUnderUpperLayer(CBaseLayer* pUULayer = NULL);
	// 매개변수 pUULayer의 상위 계층을 현재계층으로, 현재계층의 하위 계층을 pUULayer로 설정

	void SetUpperUnderLayer(CBaseLayer* pUULayer = NULL);
	// 매개변수 pUULayer의 하위 계층을 현재계층으로 , 현재계층의 상위 계층을 pUULayer로 설정

	void SetUnderLayer(CBaseLayer* pUnderLayer = NULL);
	// 현재 계층의 하위 계층을 설정함

	void SetUpperLayer(CBaseLayer* pUpperLayer = NULL);
	// 현재 계층의 상위 계층을 설정함

	CBaseLayer(char* pName = NULL);	// 디폴트 매개변수를 가지는 생성자
	virtual ~CBaseLayer();

	// param : unsigned char*	- the data of the upperlayer
	//         int				- the length of data
	// 하위 계층으로 data를 보내는 함수
	virtual	BOOL Send(unsigned char* ppayload, int nlength, int dev_num) { return FALSE; }
	virtual	BOOL Send(int command , int dev_num, int resend) { return FALSE; }
	// param : unsigned char*	- the data of the underlayer
	// 하위 계층에서 data를 받아오는 함수
	virtual	BOOL Receive(unsigned char* ppayload, int dev_num) { return FALSE; } 
	virtual	BOOL Receive() { return FALSE ; }
	//Threads
	static HANDLE File;
	static HANDLE Transfer;
	static HANDLE Sending;

protected:
	char* m_pLayerName;	// Current Layer Name
	CBaseLayer* mp_UnderLayer;	// UnderLayer pointer
	CBaseLayer*	mp_aUpperLayer[ MAX_LAYER_NUMBER ];	// UpperLayer pointer
	int	m_nUpperLayerCount;	// UpperLayer Count
};
//unsigned short htons(unsigned short x){return (((x & 0x00ff) << 8) | ((x & 0xff00) >> 8)); }

#endif // !defined(AFX_BASELAYER_H__041C5A07_23A9_4CBC_970B_8743460A7DA9__INCLUDED_)
