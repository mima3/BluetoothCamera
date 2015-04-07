#include "stdafx.h"
#include "BluetoothServer.h"
#include <WinSock2.h>
#include <bthsdpdef.h>
#include <BluetoothAPIs.h>
#include "AutoLock.h"
#include "BufferUtil.h"
#include "BluetoothCameraNetInterface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "irprops.lib")

BluetoothServerError bsError;
static pthread_mutex_t mutexlock = PTHREAD_MUTEX_INITIALIZER;
int nextid = 1;
std::map<int, ReadwriteThreadData> m_mapReadWriteThread;
SOCKET m_connectionSocket;


CBluetoothServer::CBluetoothServer()
{
}


CBluetoothServer::~CBluetoothServer()
{
}

WCHAR* GetLastErrorMessage(DWORD last_error)
{
	static WCHAR errmsg[512];

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, last_error, 0, errmsg, 511, NULL)) {
		/* if we fail, call ourself to find out why and return that error */
		wsprintf(errmsg, L"Format message failed with 0x%x\n", GetLastError());
	}
	return errmsg;
}


void* CBluetoothServer::readThread(void* param) {
	ReadwriteThreadParam* pParamRW = (ReadwriteThreadParam*)param;
	char buffer[1024];
	pthread_t pid = pthread_self();
	while (true) {
		memset(buffer, 0, sizeof(buffer));
		int recvSize = recv(pParamRW->socket, (char*)buffer, sizeof(buffer), 0);
		if (recvSize == -1) {
			// 自分でキャンセル.
			return NULL;
		}
		const ReadwriteThreadData* data = findReadWriteThreadByThread(pid);
		if (recvSize == 0) {
			// close
			data->pCallback->OnConnectionClosed(pParamRW->saddr);
			removeReadWriteThread(data->id);
			return NULL;
		}
		data->pCallback->OnReceivedCallback(pParamRW->socket, pParamRW->saddr, &buffer[0], recvSize);
		pthread_testcancel();
	}
	return NULL;
}

/**
 * 読み書き用スレッドの追加
 * @param [in] pthread_t thread スレッド識別子
 * @param [in] ReadwriteThreadParam paramRW ReadWriteスレッドのパラメータ
 * @param [in] CBluetootheServerCallback* pCallBack コールバックの定義
 */
int CBluetoothServer::addReadWriteThread(pthread_t thread, ReadwriteThreadParam* paramRW, CBluetootheServerCallback* pCallBack) {
	CAutoLock lock(&mutexlock);

	ReadwriteThreadData data;
	data.thread = thread;
	data.param = paramRW;
	data.pCallback = pCallBack;
	data.id = nextid++;
	m_mapReadWriteThread[data.id] = data;
	return data.id;
}


/**
 * ReadWriteThreadDataの取得
 * @param [in] id ID
 * @return ReadwriteThreadData
 */
const ReadwriteThreadData* CBluetoothServer::getReadWriteThread(int id) {
	CAutoLock lock(&mutexlock);
	ReadwriteThreadData* ret = &m_mapReadWriteThread[id];
	return ret;
}

/**
 * スレッド識別子によりReadWriteThreadDataの取得
 * @param [in] pthread_t thread
 * @return ReadwriteThreadData
 */
const ReadwriteThreadData* CBluetoothServer::findReadWriteThreadByThread(pthread_t thread) {
	CAutoLock lock(&mutexlock);
	ReadwriteThreadData* ret = NULL;
	for (map<int, ReadwriteThreadData>::iterator itr = m_mapReadWriteThread.begin();
		 itr != m_mapReadWriteThread.end(); ++itr) {
		if (itr->second.thread.p == thread.p && itr->second.thread.x == thread.x) {
			ret = &itr->second;
			break;
		}
	}
	return ret;
}

/**
* デバイスのアドレスによりReadWriteThreadDataの取得
* @param [in] UINT64 addr
* @return ReadwriteThreadData
*/
const ReadwriteThreadData* CBluetoothServer::findReadWriteThreadByAddress(UINT64 addr) {
	CAutoLock lock(&mutexlock);
	ReadwriteThreadData* ret = NULL;
	for (map<int, ReadwriteThreadData>::iterator itr = m_mapReadWriteThread.begin();
		itr != m_mapReadWriteThread.end(); ++itr) {
		if (GET_SAP(itr->second.param->saddr.btAddr) == addr) {
			ret = &itr->second;
			break;
		}
	}
	return ret;
}

SOCKET CBluetoothServer::GetSocketByAddress(UINT64 addr) {
	const ReadwriteThreadData* pData = findReadWriteThreadByAddress(addr);
	return pData->param->socket;
}


void CBluetoothServer::removeReadWriteThread(int id) {
	CAutoLock lock(&mutexlock);
	delete m_mapReadWriteThread[id].param;
	m_mapReadWriteThread.erase(id);
}

/**
 * コネクション管理用のスレッド
 * @param [in] void* param ConnectionThreadParamへのポインタ
 */
void* CBluetoothServer::connectitonThread(void *param)
{
	ConnectionThreadParam* connParam = (ConnectionThreadParam*)param;
	GUID serviceId = connParam->guid;
	CBluetootheServerCallback* pCallback = connParam->pCallback;
	pthread_t self;
	self = pthread_self();
	unsigned long long id = pthread_getunique_np(self);

	m_connectionSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

	if (m_connectionSocket == INVALID_SOCKET)
	{
		bsError.errorcode = ::GetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}
	WSAPROTOCOL_INFO protocolInfo;
	int protocolInfoSize = sizeof(protocolInfo);

	if (getsockopt(m_connectionSocket, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&protocolInfo, &protocolInfoSize) != 0)
	{
		bsError.errorcode = WSAGetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}
	SOCKADDR_BTH address;
	address.addressFamily = AF_BTH;
	address.btAddr = 0;
	address.serviceClassId = GUID_NULL;
	address.port = BT_PORT_ANY;
	sockaddr *pAddr = (sockaddr*)&address;
	if (0 != bind(m_connectionSocket, pAddr, sizeof(SOCKADDR_BTH)))
	{
		bsError.errorcode = WSAGetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}

	int length = sizeof(SOCKADDR_BTH);
	getsockname(m_connectionSocket, (sockaddr*)&address, &length);
	wprintf(L"Local Bluetooth device is %04x%08x \nServer channel = %d\n", GET_NAP(address.btAddr), GET_SAP(address.btAddr), address.port);

	int size = sizeof(SOCKADDR_BTH);
	if (0 != getsockname(m_connectionSocket, pAddr, &size))
	{
		bsError.errorcode = GetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}
	if (0 != listen(m_connectionSocket, 10))
	{
		bsError.errorcode = GetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}
	WSAQUERYSET service;
	memset(&service, 0, sizeof(service));
	service.dwSize = sizeof(service);
	service.lpszServiceInstanceName = _T("Accelerometer Data...");
	service.lpszComment = _T("Pushing data to PC");


	service.lpServiceClassId = &serviceId;
	service.dwNumberOfCsAddrs = 1;
	service.dwNameSpace = NS_BTH;

	CSADDR_INFO csAddr;
	memset(&csAddr, 0, sizeof(csAddr));
	csAddr.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
	csAddr.LocalAddr.lpSockaddr = pAddr;
	csAddr.iSocketType = SOCK_STREAM;
	csAddr.iProtocol = BTHPROTO_RFCOMM;
	service.lpcsaBuffer = &csAddr;

	if (0 != WSASetService(&service, RNRSERVICE_REGISTER, 0))
	{
		bsError.errorcode = GetLastError();
		bsError.message = GetLastErrorMessage(bsError.errorcode);
		connParam->pCallback->OnErrorCallback(bsError);
		pthread_exit(&bsError);
		return NULL;
	}

	printf("\nBefore accept.........");

	while (true) {
		SOCKADDR_BTH sab2;
		int ilen = sizeof(sab2);
		SOCKET s2 = accept(m_connectionSocket, (sockaddr*)&sab2, &ilen);
		if (s2 == INVALID_SOCKET)
		{
			bsError.errorcode = WSAGetLastError();
			bsError.message = GetLastErrorMessage(bsError.errorcode);
			connParam->pCallback->OnErrorCallback(bsError);
			pthread_exit(&bsError);
			return NULL;
		}
		ReadwriteThreadParam* paramRW = new ReadwriteThreadParam();
		paramRW->saddr = sab2;
		paramRW->socket = s2;
		pthread_t thread;

		if (pCallback->OnConnectedCallback(s2, sab2)) {
			int ret = pthread_create(&thread, NULL, CBluetoothServer::readThread, paramRW);
			if (ret) {
				bsError.errorcode = ret;
				bsError.message = GetLastErrorMessage(bsError.errorcode);
				connParam->pCallback->OnErrorCallback(bsError);
				pthread_exit(&bsError);
				return NULL;
			}
			CBluetoothServer::addReadWriteThread(thread, paramRW, pCallback);
		}
		pthread_testcancel();
	}
	return NULL;
}

/**
 * サービスの開始
 * @param [in] guid サービスのGUID
 * @param [in] CBluetootheServerCallback コールバックへのポインタ
 * @return true 成功, false 失敗
 */
bool CBluetoothServer::StartService(const GUID& guid, CBluetootheServerCallback* callback) {
	WORD wVersionRequested = 0x202;
	WSADATA m_data;
	unsigned long ret = WSAStartup(wVersionRequested, &m_data);
	if (ret != 0) {
		return false;
	}

	this->m_connParam.guid = guid;
	this->m_connParam.pCallback = callback;

	ret = pthread_create(&this->m_connThread, NULL, CBluetoothServer::connectitonThread, &this->m_connParam);
	if (ret) {
		return false;
	}
	return true;
}

/**
 * サービスの停止
 */
void CBluetoothServer::StopService() {
	pthread_cancel(this->m_connThread);
	closesocket(m_connectionSocket);
	pthread_join(this->m_connThread, NULL);
	{
		CAutoLock lock(&mutexlock);
		for (map <int, ReadwriteThreadData>::iterator itr = m_mapReadWriteThread.begin();
			itr != m_mapReadWriteThread.end(); ++itr) {
			pthread_cancel(itr->second.thread);
			closesocket(itr->second.param->socket);
			pthread_join(itr->second.thread, NULL);
			delete itr->second.param;
		}
	}
	m_mapReadWriteThread.clear();

	WSACleanup();
}

/**
 * ソケットの切断
 * @param[in] addr デバイスのアドレス
 */
void CBluetoothServer::CloseSocket(UINT64 addr) {
	const ReadwriteThreadData* pData = CBluetoothServer::findReadWriteThreadByAddress(addr);
	closesocket(pData->param->socket);
}
