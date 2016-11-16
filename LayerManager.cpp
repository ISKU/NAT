#include "StdAfx.h"
#include "LayerManager.h"

CLayerManager::CLayerManager()	// Construction
	: m_nLayerCount( 0 ), // Layer의 수는 0 으로 초기화
	mp_sListHead( NULL ),			
	mp_sListTail( NULL ),
	m_nTop( -1 )
{
}

CLayerManager::~CLayerManager()
{
}

// Layer를 추가
void CLayerManager::AddLayer(CBaseLayer *pLayer)
{
	mp_aLayers[m_nLayerCount++] = pLayer; // Layer를 저장하는 장소인 aLayers에 추가 , Layer의 수 증가!
}

// nindex에 해당하는 Layer를 반환
CBaseLayer* CLayerManager::GetLayer(int nindex)
{
	return mp_aLayers[nindex];
}

// pName에 해당하는 Layer를 반환
CBaseLayer* CLayerManager::GetLayer(char *pName)
{
	for (int i = 0 ; i < m_nLayerCount; i++) {
		CString stName(pName);
		if ( !stName.Compare(mp_aLayers[i]->GetLayerName()))
			return mp_aLayers[i];
	}

	return NULL ;
}

// Layer를 연결
void CLayerManager::ConnectLayers(char *pcList)	// pcList - 연결된 관계를 나타내는거
{ // "File ( *Ethernet  ( *ChatApp ( *ChatDlg ) ) )"
	MakeList(pcList);// Node를 만들고 Node를 연결하는 함수
	LinkLayer(mp_sListHead); // MakeList의 AddNode에서 mp_sListHead("File node")를 지정함
	// pcList대로 실제로 Layer들을 연결
}

// pcList로 들어온 문자열 대로 node들을 연결한다.
void CLayerManager::MakeList(char *pcList)
{
	/* pcList가 가리키는 값이 없어지게 되면 strchr에 의해 공백문자를 찾을수 없게 되면 
	strchr()은 NULL을 리턴하게 되고 거기에 +1을 하게 되면 pcList = 0x00000001 값을 가지게 된다. */
	while(pcList != (char*) 0x01) {									
		char sBuff[100]; // pcList의 하나의 문자열 들을 임시로 저장하는 Buffer 설정 
		sscanf(pcList, "%s", sBuff); // pcList가 가리키는 문자열을 읽어 들여 공백문자가 나타난기 전까지의 문자열을
		// sBuff에 저장한다.
		pcList = strchr(pcList, ' ') + 1;	// 공백문자 다음 문자의 주소를 시작 번지로 가짐
		// pcList에서 공백을 검사하고 공백문자가 있는 곳의 주소를 리턴한다. 공백문자가 없을 경우 NULL을 리턴한다.
		PNODE pNode = AllocNode(sBuff) ;	// sBuff에 해당하는 문자열의 이름을 가진 노드를 만듦
		AddNode(pNode) ;					// 노드를 추가하고 연결한다.
	}
}

// pcName으로 node를 만들고 초기화 한다.
CLayerManager::PNODE CLayerManager::AllocNode(char* pcName)	// pcName은 Layer의 이름임
{
	PNODE node = new NODE;
	ASSERT(node) ;		// node가 거짓이면 계속할 것인지 중단할 것인지를 묻는다.

	strcpy(node->token, pcName) ;		// node의 token 배열에 Layer 이름을 넣음 
	node->next = NULL ;					// node의 next 는 NULL로 저장

	return node ;						// 노드를 리턴
}

// Node를 추가하고 노드를 연결한다.
void CLayerManager::AddNode(PNODE pNode)
{
	if ( !mp_sListHead ) // Null이면 - 처음 node를 만들경우
		mp_sListHead = mp_sListTail = pNode; // 처음 node를 만드는 경우 Head 와 Tail pNode로 지정
	else {
		mp_sListTail->next = pNode;		// 처음이 아닐경우 tail 의 next를 매개변수로 들어온 pNode로 지정
		mp_sListTail = pNode;				// tail을 매개변수로 들어온 pNode로 지정
		// 처음 tail은 file이고 file의 next를 pNode로 해주고 tail을 다시 pNode로 해야 계속해서 이어질수 있다.
	}
}

// Stack에 대한 Push
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

// Stack에 대한 Pop
CBaseLayer* CLayerManager::Pop()
{
	if (m_nTop < 0) {
#ifdef _DEBUG
		TRACE( "The Stack is empty.. so cannot run the pop operation.. \n" );
#endif
		return NULL ;
	}

	CBaseLayer* pLayer = mp_Stack[m_nTop]; // Top에 해당 하는 Layer를 임시 객체에 저장하고
	mp_Stack[m_nTop] = NULL; // Top에 해당 하는 곳을 NUll로 만듦
	m_nTop--; // Top의 위치를 감소시킴

	return pLayer; // 임시 객체에 저장한것을 리턴함
}

// Top에 해당하는 Layer를 리턴
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

// Node를 연결함 - 여기서 pNode == mp_sListHead == File
// mp_sListHead는 처음 노드가 만들어 질때 , 처음 node(File)이 Head Node 이다. (File-Ethernet-Chatapp-ChaDlg)순으로 만들어짐
// "File ( *Ethernet  ( *ChatApp ( *ChatDlg ) ) )" 이렇게 일부러 공백을 두고 '(' , ')' 을 넣어준다.
void CLayerManager::LinkLayer(PNODE pNode)		// pNode == File
{
	CBaseLayer* pLayer = NULL;

	while (pNode) {// pNode != Null - 당연히 NULL이 아님 , 처음 pNode->token == File , 다음은 pNode->token == ( 이다.
		if (!pLayer) // pLayer == NULL - 처음은 무조건 pLayer이 NULL 이다.
			pLayer = GetLayer(pNode->token); 	// pNode->token(pName)(mp_sListHead)에 해당하는 Layer를 받아옴
		else {
			if (*pNode->token == '(')
				Push(pLayer); // stack에 이전의 문자열에 해당하는 Layer를 push
			// mp_stacks[File , Ethernet , ChatApp , ChatDlg] 순으로 되어있음
			else if (*pNode->token == ')')
				Pop(); // stack에서 이전의 문자열에 해당하는 Layer pop
			else { // ( , ) 가 아닐경우(Layer 이름일 경우) 실행 되는 부분
				char cMode = *pNode->token; // * , + , - 를 저장하는 변수
				char* pcName = pNode->token + 1; 	// * , + , - 를 제외한 문자열을 가리키는 변수

				pLayer = GetLayer(pcName); // Layer를 불러옴

				switch(cMode) {
				case '*': 
					Top()->SetUpperUnderLayer(pLayer); 
					break;	// 양방향 연결
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

// Layer들을 제거함
void CLayerManager::DeAllocLayer()
{
	for (int i = 0; i < this->m_nLayerCount; i++)
		delete this->mp_aLayers[i];
}