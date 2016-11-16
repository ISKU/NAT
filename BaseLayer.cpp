#include "StdAfx.h"
#include "BaseLayer.h"

HANDLE CBaseLayer::File = CreateSemaphore(NULL, 0, 1, NULL);
HANDLE CBaseLayer::Transfer = CreateSemaphore(NULL, 0, 990, NULL);
HANDLE CBaseLayer::Sending = CreateSemaphore(NULL, 1, 1, NULL);

CBaseLayer::CBaseLayer(char* pName) // ����Ʈ �Ű����� pName�� ���� ������ 
	: m_nUpperLayerCount(0), // ������ ȣ��� m_nUpperLayerCount = 0 ���� �ʱ�ȭ
	mp_UnderLayer(NULL) // mp_UnderLayer = NULL�� �ʱ�ȭ
{
	m_pLayerName = pName; // �Ű������� ������ m_pLayerName(������ �̸�) = pName ���� ����
}

CBaseLayer::~CBaseLayer()
{
}

// �Ű����� pUULayer�� ���� ������ �����������, ��������� ���� ������ pUULayer�� ����
void CBaseLayer::SetUnderUpperLayer(CBaseLayer *pUULayer)
{
	if (!pUULayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUnderUpperLayer] The variable , 'pUULayer' is NULL" );
		#endif
		return; // pUULayer�� NULL �̸� �Լ� ����!		
	}

	this->mp_UnderLayer = pUULayer; // ���� ������ ���� �������� pUULayer ����
	pUULayer->SetUpperLayer(this); // pUULayer�� ���� ���� ������ ���� SetUpperLayer() �Լ� ȣ��! ���� ������ ���� �����̴�.
}

// �Ű����� pUULayer�� ���� ������ ����������� , ��������� ���� ������ pUULayer�� ����
void CBaseLayer::SetUpperUnderLayer(CBaseLayer *pUULayer)
{
	if (!pUULayer) { // if the pointer is null,
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUpperUnderLayer] The variable , 'pUULayer' is NULL" );
		#endif
		return; // pUULayer�� NULL �̸� �Լ� ����!		
	}

	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUULayer; // ���� ������ ���� �������� pUULayer ����
	pUULayer->SetUnderLayer(this); // pUULayer�� ���� ���� ������ ���� SetUpperLayer() �Լ� ȣ�� ���� ������ ������ �����̴�.
}

// �Ű����� pUpperLayer�� ȣ���� ������ ���� �������� ����
void CBaseLayer::SetUpperLayer(CBaseLayer *pUpperLayer)
{
	if (!pUpperLayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUpperLayer] The variable , 'pUpperLayer' is NULL" );
		#endif
		return; // pUpperLayer�� NULL �̸� �Լ� ����!			
	}

	// UpperLayer is added.
	this->mp_aUpperLayer[m_nUpperLayerCount++] = pUpperLayer; // ���� ������ ���� �������� pUpperLayer�� ����
}
// �Ű����� pUnderLayer�� ȣ���� ������ ���� �������� ����
void CBaseLayer::SetUnderLayer(CBaseLayer *pUnderLayer)
{
	if (!pUnderLayer) { // if the pointer is null, 
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::SetUnderLayer] The variable , 'pUnderLayer' is NULL\n" );
		#endif
		return; // pUnderLayer�� NULL �̸� �Լ� ����!
	}

	// UnderLayer assignment..
	this->mp_UnderLayer = pUnderLayer ;	// ���� ������ ���� �������� pUnderLayer�� ����
}

// �Ű����� nindex�� �ش��ϴ� ���� ������ ���� ������ ��ȯ
CBaseLayer* CBaseLayer::GetUpperLayer(int nindex)
{
	if (nindex < 0	// nindex�� 0���� �۰ų�
		|| nindex > m_nUpperLayerCount // ���� ������ ������ nindex���� ũ�ų�
		||m_nUpperLayerCount < 0) // ���� ������ ������
	{
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::GetUpperLayer] There is no UpperLayer in Array..\n" );
		#endif 
		return NULL; // NULL�� ����
	}

	return mp_aUpperLayer[nindex]; // nindex�� �ش��ϴ� ���� ���� ����
}

// ��������� ���� ������ ��ȯ
CBaseLayer* CBaseLayer::GetUnderLayer()
{
	if (!mp_UnderLayer) { // ���� ������ ������
		#ifdef _DEBUG
			TRACE( "[CBaseLayer::GetUnderLayer] There is not a UnerLayer..\n" );
		#endif 
		return NULL; // NULL�� ����
	}

	return mp_UnderLayer; // ������ ���� ���� ����
}

// ���� ������ �̸��� ��ȯ
char* CBaseLayer::GetLayerName()
{
	return m_pLayerName ; 
}
