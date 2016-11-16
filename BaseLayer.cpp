#include "StdAfx.h"
#include "BaseLayer.h"

HANDLE CBaseLayer::File = CreateSemaphore(NULL, 0, 1, NULL);
HANDLE CBaseLayer::Transfer = CreateSemaphore(NULL, 0, 990, NULL);
HANDLE CBaseLayer::Sending = CreateSemaphore(NULL, 1, 1, NULL);

CBaseLayer::CBaseLayer(char* pName) // 디폴트 매개변수 pName을 갖는 생성자 
	: m_nUpperLayerCount(0), // 생성자 호출시 m_nUpperLayerCount = 0 으로 초기화
	mp_UnderLayer(NULL) // mp_UnderLayer = NULL로 초기화
{
	m_pLayerName = pName; // 매개변수가 잇으면 m_pLayerName(계층의 이름) = pName 으로 설정
}

CBaseLayer::~CBaseLayer()
{
}

// 매개변수 pUULayer의 상위 계층을 현재계층으로, 현재계층의 하위 계층을 pUULayer로 설정
void CBaseLayer::SetUnderUpperLayer(CBaseLayer *pUULayer)
{
	if (!pUULayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUnderUpperLayer] The variable , 'pUULayer' is NULL" );
		#endif
		return; // pUULayer이 NULL 이면 함수 종료!		
	}

	this->mp_UnderLayer = pUULayer; // 현재 계층의 하위 계층으로 pUULayer 설정
	pUULayer->SetUpperLayer(this); // pUULayer의 상위 계층 설정을 위해 SetUpperLayer() 함수 호출! 상위 계층은 현재 계층이다.
}

// 매개변수 pUULayer의 하위 계층을 현재계층으로 , 현재계층의 상위 계층을 pUULayer로 설정
void CBaseLayer::SetUpperUnderLayer(CBaseLayer *pUULayer)
{
	if (!pUULayer) { // if the pointer is null,
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUpperUnderLayer] The variable , 'pUULayer' is NULL" );
		#endif
		return; // pUULayer이 NULL 이면 함수 종료!		
	}

	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; // 현재 계층의 상위 계층으로 pUULayer 설정
	pUULayer->SetUnderLayer(this); // pUULayer의 하위 계층 설정을 위해 SetUpperLayer() 함수 호출 하위 계층은 현재의 계층이다.
}

// 매개변수 pUpperLayer를 호출한 계층의 상위 계층으로 설정
void CBaseLayer::SetUpperLayer(CBaseLayer *pUpperLayer)
{
	if (!pUpperLayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUpperLayer] The variable , 'pUpperLayer' is NULL" );
		#endif
		return; // pUpperLayer이 NULL 이면 함수 종료!			
	}

	// UpperLayer is added.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUpperLayer; // 현재 계층의 상위 계층으로 pUpperLayer를 설정
}
// 매개변후 pUnderLayer를 호출한 계층의 하위 계층으로 설정
void CBaseLayer::SetUnderLayer(CBaseLayer *pUnderLayer)
{
	if (!pUnderLayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUnderLayer] The variable , 'pUnderLayer' is NULL\n" );
		#endif
		return; // pUnderLayer이 NULL 이면 함수 종료!
	}

	// UnderLayer assignment..
	this->mp_UnderLayer = pUnderLayer ;	// 현재 계층의 하위 계층으로 pUnderLayer를 설정
}

// 매개변수 nindex에 해당하는 현재 계층의 상위 계층을 반환
CBaseLayer* CBaseLayer::GetUpperLayer(int nindex)
{
	if (nindex < 0	// nindex가 0보다 작거나
		|| nindex > m_nUpperLayerCount // 상위 계층의 수보다 nindex값이 크거나
		||m_nUpperLayerCount < 0) // 상위 계층이 없을시
	{
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::GetUpperLayer] There is no UpperLayer in Array..\n" );
		#endif 
		return NULL; // NULL을 리턴
	}

	return mp_aUpperLayer[nindex]; // nindex에 해당하는 상위 계층 리턴
}

// 현재계층의 하위 계층을 반환
CBaseLayer* CBaseLayer::GetUnderLayer()
{
	if (!mp_UnderLayer) { // 하위 계층이 없으면
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::GetUnderLayer] There is not a UnerLayer..\n" );
		#endif 
		return NULL; // NULL을 리턴
	}

	return mp_UnderLayer; // 있으면 하위 계층 리턴
}

// 현재 계층의 이름을 반환
char* CBaseLayer::GetLayerName()
{
	return m_pLayerName ; 
}
