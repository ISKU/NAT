#pragma once
#include "BaseLayer.h"

class CLayerManager  
{
private:
	typedef struct _NODE {
		char token[50] ;
		struct _NODE* next ;
	} NODE, *PNODE ;		
	// 각 layer의 이름을 나타내는 구조체(다음을 저장하기 위해 구조체로 표현)
	// 그니깐 이름들이 서로 연결된 상태를 나타내기 위한것이라고 보면됨

public:
	void DeAllocLayer(); // layer를 지우기
	void ConnectLayers(char* pcList); // Layer를 연결
	CBaseLayer*	GetLayer(char* pName); // Layer를 반환
	CBaseLayer*	GetLayer(int nindex); // Layer를 반환
	void AddLayer(CBaseLayer* pLayer);	// Layer를 추가하기

	CLayerManager();
	virtual ~CLayerManager();

private:
	// about stack...
	int m_nTop;	// Stack의 Top를 나타냄
	CBaseLayer*	mp_Stack[MAX_LAYER_NUMBER];// Layer를 잠시 저장하는 Stack

	CBaseLayer* Top(); // Top에 있는 Layer를 리턴
	CBaseLayer*	Pop(); // Layer를 빼냄
	void Push(CBaseLayer* pLayer); // Layer를 추가

	PNODE mp_sListHead;	// 해당 Layer의 상위 Layer의 이름.
	PNODE mp_sListTail;	// 해당 Layer의 하위 Layer의 이름.

	// about Link Layer...
	void LinkLayer(PNODE pNode);// Layer의 이름을 연결

	inline void	AddNode(PNODE pNode); // Layer의 이름을 추가
	inline PNODE AllocNode(char* pcName); // Layer의 이름을 저장하는 Node 만들기
	void MakeList(char* pcList); // ??

	int	m_nLayerCount;	// Layer의 수
	CBaseLayer*	mp_aLayers[MAX_LAYER_NUMBER]; // Layer 객체를 연결한 배열
};