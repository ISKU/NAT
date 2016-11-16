#pragma once
#ifndef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif
#include "resource.h" // 주 기호입니다.

// CRouterApp:
// 이 클래스의 구현에 대해서는 Router.cpp을 참조하십시오.
class CRouterApp : public CWinApp
{
public:
	CRouterApp();

public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};

extern CRouterApp theApp;