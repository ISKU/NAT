#pragma once
#include "BaseLayer.h"

class CLayerManager  
{
private:
	typedef struct _NODE {
		char token[50] ;
		struct _NODE* next ;
	} NODE, *PNODE ;		
	// �� layer�� �̸��� ��Ÿ���� ����ü(������ �����ϱ� ���� ����ü�� ǥ��)
	// �״ϱ� �̸����� ���� ����� ���¸� ��Ÿ���� ���Ѱ��̶�� �����

public:
	void DeAllocLayer(); // layer�� �����
	void ConnectLayers(char* pcList); // Layer�� ����
	CBaseLayer*	GetLayer(char* pName); // Layer�� ��ȯ
	CBaseLayer*	GetLayer(int nindex); // Layer�� ��ȯ
	void AddLayer(CBaseLayer* pLayer);	// Layer�� �߰��ϱ�

	CLayerManager();
	virtual ~CLayerManager();

private:
	// about stack...
	int m_nTop;	// Stack�� Top�� ��Ÿ��
	CBaseLayer*	mp_Stack[MAX_LAYER_NUMBER];// Layer�� ��� �����ϴ� Stack

	CBaseLayer* Top(); // Top�� �ִ� Layer�� ����
	CBaseLayer*	Pop(); // Layer�� ����
	void Push(CBaseLayer* pLayer); // Layer�� �߰�

	PNODE mp_sListHead;	// �ش� Layer�� ���� Layer�� �̸�.
	PNODE mp_sListTail;	// �ش� Layer�� ���� Layer�� �̸�.

	// about Link Layer...
	void LinkLayer(PNODE pNode);// Layer�� �̸��� ����

	inline void	AddNode(PNODE pNode); // Layer�� �̸��� �߰�
	inline PNODE AllocNode(char* pcName); // Layer�� �̸��� �����ϴ� Node �����
	void MakeList(char* pcList); // ??

	int	m_nLayerCount;	// Layer�� ��
	CBaseLayer*	mp_aLayers[MAX_LAYER_NUMBER]; // Layer ��ü�� ������ �迭
};