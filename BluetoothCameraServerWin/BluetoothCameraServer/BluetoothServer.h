#pragma once
#include <pthread.h>
#include <ws2bth.h>
#include <map>

using namespace std;

struct BluetoothServerError {
	unsigned long errorcode;
	WCHAR* message;
};

class CBluetootheServerCallback {
public:
	virtual void OnErrorCallback(const BluetoothServerError& error) { };
	virtual bool OnConnectedCallback(SOCKET socket, SOCKADDR_BTH saddr) { return true; };
	virtual void OnReceivedCallback(SOCKET socket, SOCKADDR_BTH saddr, char* data, int recvSize) { };
	virtual void OnConnectionClosed(SOCKADDR_BTH saddr) { };
};

struct ReadwriteThreadParam {
	SOCKADDR_BTH saddr;
	SOCKET socket;
};

struct ReadwriteThreadData {
	int id;
	ReadwriteThreadParam* param;
	pthread_t thread;
	CBluetootheServerCallback* pCallback;
};

struct ConnectionThreadParam{
	GUID guid;
	CBluetootheServerCallback* pCallback;
};

/**
 * @class CBluetoothServer
 * @brief Bluetoothのサーバー処理
 */
class CBluetoothServer
{
public:
	CBluetoothServer();
	~CBluetoothServer();
	bool StartService(const GUID& guid, CBluetootheServerCallback* callback);
	void StopService();
	void CloseSocket(UINT64 addr);
private:
	pthread_t m_connThread;
	ConnectionThreadParam m_connParam;


	static int addReadWriteThread(pthread_t, ReadwriteThreadParam*, CBluetootheServerCallback*);
	static const ReadwriteThreadData* getReadWriteThread(int);
	static const ReadwriteThreadData* findReadWriteThreadByThread(pthread_t);
	static const ReadwriteThreadData* findReadWriteThreadByAddress(UINT64 addr);
	static void removeReadWriteThread(int);

	static void* readThread(void* param);
	static void* connectitonThread(void* param);
};

