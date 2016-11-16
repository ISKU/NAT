#if !defined(AFX_BASELAYER_H__041C5A07_23A9_4CBC_970B_8743460A7DA9__INCLUDED_)
#define AFX_BASELAYER_H__041C5A07_23A9_4CBC_970B_8743460A7DA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"

class CBaseLayer  
{
public:
	char* GetLayerName(); // ������ �̸��� ������ �Լ�
	CBaseLayer* GetUnderLayer(); // ���� ������ ������ �Լ�
	CBaseLayer* GetUpperLayer(int nindex); // nindex�� �ش� �ϴ� ���� ������ ������ �Լ�

	void SetUnderUpperLayer(CBaseLayer* pUULayer = NULL);
	// �Ű����� pUULayer�� ���� ������ �����������, ��������� ���� ������ pUULayer�� ����

	void SetUpperUnderLayer(CBaseLayer* pUULayer = NULL);
	// �Ű����� pUULayer�� ���� ������ ����������� , ��������� ���� ������ pUULayer�� ����

	void SetUnderLayer(CBaseLayer* pUnderLayer = NULL);
	// ���� ������ ���� ������ ������

	void SetUpperLayer(CBaseLayer* pUpperLayer = NULL);
	// ���� ������ ���� ������ ������

	CBaseLayer(char* pName = NULL);	// ����Ʈ �Ű������� ������ ������
	virtual ~CBaseLayer();

	// param : unsigned char*	- the data of the upperlayer
	//         int				- the length of data
	// ���� �������� data�� ������ �Լ�
	virtual	BOOL Send(unsigned char* ppayload, int nlength, int dev_num) { return FALSE; }
	virtual	BOOL Send(int command , int dev_num, int resend) { return FALSE; }
	// param : unsigned char*	- the data of the underlayer
	// ���� �������� data�� �޾ƿ��� �Լ�
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
