#pragma once
#ifndef __AFXWIN_H__
#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif
#include "resource.h" // �� ��ȣ�Դϴ�.

// CRouterApp:
// �� Ŭ������ ������ ���ؼ��� Router.cpp�� �����Ͻʽÿ�.
class CRouterApp : public CWinApp
{
public:
	CRouterApp();

public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};

extern CRouterApp theApp;