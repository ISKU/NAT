#include "StdAfx.h"
#include "LayerManager.h"

CLayerManager::CLayerManager()	// Construction
	: m_nLayerCount( 0 ), // Layer�� ���� 0 ���� �ʱ�ȭ
	mp_sListHead( NULL ),			
	mp_sListTail( NULL ),
	m_nTop( -1 )
{
}

CLayerManager::~CLayerManager()
{
}

// Layer�� �߰�
void CLayerManager::AddLayer(CBaseLayer *pLayer)
{
	mp_aLayers[m_nLayerCount++] = pLayer; // Layer�� �����ϴ� ����� aLayers�� �߰� , Layer�� �� ����!
}

// nindex�� �ش��ϴ� Layer�� ��ȯ
CBaseLayer* CLayerManager::GetLayer(int nindex)
{
	return mp_aLayers[nindex];
}

// pName�� �ش��ϴ� Layer�� ��ȯ
CBaseLayer* CLayerManager::GetLayer(char *pName)
{
	for (int i = 0 ; i < m_nLayerCount; i++) {
		CString stName(pName);
		if ( !stName.Compare(mp_aLayers[i]->GetLayerName()))
			return mp_aLayers[i];
	}

	return NULL ;
}

// Layer�� ����
void CLayerManager::ConnectLayers(char *pcList)	// pcList - ����� ���踦 ��Ÿ���°�
{ // "File ( *Ethernet  ( *ChatApp ( *ChatDlg ) ) )"
	MakeList(pcList);// Node�� ����� Node�� �����ϴ� �Լ�
	LinkLayer(mp_sListHead); // MakeList�� AddNode���� mp_sListHead("File node")�� ������
	// pcList��� ������ Layer���� ����
}

// pcList�� ���� ���ڿ� ��� node���� �����Ѵ�.
void CLayerManager::MakeList(char *pcList)
{
	/* pcList�� ����Ű�� ���� �������� �Ǹ� strchr�� ���� ���鹮�ڸ� ã���� ���� �Ǹ� 
	strchr()�� NULL�� �����ϰ� �ǰ� �ű⿡ +1�� �ϰ� �Ǹ� pcList = 0x00000001 ���� ������ �ȴ�. */
	while(pcList != (char*) 0x01) {									
		char sBuff[100]; // pcList�� �ϳ��� ���ڿ� ���� �ӽ÷� �����ϴ� Buffer ���� 
		sscanf(pcList, "%s", sBuff); // pcList�� ����Ű�� ���ڿ��� �о� �鿩 ���鹮�ڰ� ��Ÿ���� �������� ���ڿ���
		// sBuff�� �����Ѵ�.
		pcList = strchr(pcList, ' ') + 1;	// ���鹮�� ���� ������ �ּҸ� ���� ������ ����
		// pcList���� ������ �˻��ϰ� ���鹮�ڰ� �ִ� ���� �ּҸ� �����Ѵ�. ���鹮�ڰ� ���� ��� NULL�� �����Ѵ�.
		PNODE pNode = AllocNode(sBuff) ;	// sBuff�� �ش��ϴ� ���ڿ��� �̸��� ���� ��带 ����
		AddNode(pNode) ;					// ��带 �߰��ϰ� �����Ѵ�.
	}
}

// pcName���� node�� ����� �ʱ�ȭ �Ѵ�.
CLayerManager::PNODE CLayerManager::AllocNode(char* pcName)	// pcName�� Layer�� �̸���
{
	PNODE node = new NODE;
	ASSERT(node) ;		// node�� �����̸� ����� ������ �ߴ��� �������� ���´�.

	strcpy(node->token, pcName) ;		// node�� token �迭�� Layer �̸��� ���� 
	node->next = NULL ;					// node�� next �� NULL�� ����

	return node ;						// ��带 ����
}

// Node�� �߰��ϰ� ��带 �����Ѵ�.
void CLayerManager::AddNode(PNODE pNode)
{
	if ( !mp_sListHead ) // Null�̸� - ó�� node�� ������
		mp_sListHead = mp_sListTail = pNode; // ó�� node�� ����� ��� Head �� Tail pNode�� ����
	else {
		mp_sListTail->next = pNode;		// ó���� �ƴҰ�� tail �� next�� �Ű������� ���� pNode�� ����
		mp_sListTail = pNode;				// tail�� �Ű������� ���� pNode�� ����
		// ó�� tail�� file�̰� file�� next�� pNode�� ���ְ� tail�� �ٽ� pNode�� �ؾ� ����ؼ� �̾����� �ִ�.
	}
}

// Stack�� ���� Push
void CLayerManager::Push(CBaseLayer *pLayer)
{
	if (m_nTop >= MAX_LAYER_NUMBER) {
#ifdef _DEBUG
		TRACE( "The Stack is full.. so cannot run the push operation.. \n" );
#endif
		return ;
	}

	mp_Stack[++m_nTop] = pLayer;
}

// Stack�� ���� Pop
CBaseLayer* CLayerManager::Pop()
{
	if (m_nTop < 0) {
#ifdef _DEBUG
		TRACE( "The Stack is empty.. so cannot run the pop operation.. \n" );
#endif
		return NULL ;
	}

	CBaseLayer* pLayer = mp_Stack[m_nTop]; // Top�� �ش� �ϴ� Layer�� �ӽ� ��ü�� �����ϰ�
	mp_Stack[m_nTop] = NULL; // Top�� �ش� �ϴ� ���� NUll�� ����
	m_nTop--; // Top�� ��ġ�� ���ҽ�Ŵ

	return pLayer; // �ӽ� ��ü�� �����Ѱ��� ������
}

// Top�� �ش��ϴ� Layer�� ����
CBaseLayer* CLayerManager::Top()
{
	if (m_nTop < 0) {
#ifdef _DEBUG
		TRACE( "The Stack is empty.. so cannot run the top operation.. \n" );
#endif
		return NULL;
	}

	return mp_Stack[m_nTop];
}

// Node�� ������ - ���⼭ pNode == mp_sListHead == File
// mp_sListHead�� ó�� ��尡 ����� ���� , ó�� node(File)�� Head Node �̴�. (File-Ethernet-Chatapp-ChaDlg)������ �������
// "File ( *Ethernet  ( *ChatApp ( *ChatDlg ) ) )" �̷��� �Ϻη� ������ �ΰ� '(' , ')' �� �־��ش�.
void CLayerManager::LinkLayer(PNODE pNode)		// pNode == File
{
	CBaseLayer* pLayer = NULL;

	while (pNode) {// pNode != Null - �翬�� NULL�� �ƴ� , ó�� pNode->token == File , ������ pNode->token == ( �̴�.
		if (!pLayer) // pLayer == NULL - ó���� ������ pLayer�� NULL �̴�.
			pLayer = GetLayer(pNode->token); 	// pNode->token(pName)(mp_sListHead)�� �ش��ϴ� Layer�� �޾ƿ�
		else {
			if (*pNode->token == '(')
				Push(pLayer); // stack�� ������ ���ڿ��� �ش��ϴ� Layer�� push
			// mp_stacks[File , Ethernet , ChatApp , ChatDlg] ������ �Ǿ�����
			else if (*pNode->token == ')')
				Pop(); // stack���� ������ ���ڿ��� �ش��ϴ� Layer pop
			else { // ( , ) �� �ƴҰ��(Layer �̸��� ���) ���� �Ǵ� �κ�
				char cMode = *pNode->token; // * , + , - �� �����ϴ� ����
				char* pcName = pNode->token + 1; 	// * , + , - �� ������ ���ڿ��� ����Ű�� ����

				pLayer = GetLayer(pcName); // Layer�� �ҷ���

				switch(cMode) {
				case '*': 
					Top()->SetUpperUnderLayer(pLayer); 
					break;	// ����� ����
				case '+': 
					Top( )->SetUpperLayer( pLayer ); 
					break;		 
				case '-': 
					Top( )->SetUnderLayer( pLayer ); 
					break;
				}
			}
		}

		pNode = pNode->next;
	}
}

// Layer���� ������
void CLayerManager::DeAllocLayer()
{
	for (int i = 0; i < this->m_nLayerCount; i++)
		delete this->mp_aLayers[i];
}