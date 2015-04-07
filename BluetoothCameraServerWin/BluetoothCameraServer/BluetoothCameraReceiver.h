#pragma once
#include "BluetoothServer.h"

/**
 * @struct CameraImageData
 * @breif カメラ画像データ
 */
struct CameraImageData {
	UINT64 deviceId;	//! デバイスのGUID
	size_t bufferSize;	//! バッファのサイズ
	HBITMAP hBitmap;	//! ビットマップのハンドル
};

/**
* @class CRecevicedData
* @brief 受信データのクラス
*/
class CRecevicedData {
public:
	CRecevicedData(UINT64 addr);
	~CRecevicedData();

	enum eStatus {
		kStatus_None = 0,		// 未受信
		kStatus_Receiving = 1	// 現在データを受信中
	};
	UINT64 m_Addr;
	eStatus m_status;
	size_t m_BufferSize;
	size_t m_currentPos;
	char* m_buff;
	UINT32 m_width;
	UINT32 m_height;
};

/**
 * @class CBluetoothCameraReceiver
 * @brief Bluetoothからの受信を処理して画像として変換する
 */
class CBluetoothCameraReceiver : public CBluetootheServerCallback
{
public:
	CBluetoothCameraReceiver(HWND targethWnd);
	~CBluetoothCameraReceiver();

	void OnErrorCallback(const BluetoothServerError& error);
	bool OnConnectedCallback(SOCKET socket, SOCKADDR_BTH saddr);
	void OnReceivedCallback(SOCKET socket, SOCKADDR_BTH saddr, char* data, int recvSize);
	void OnConnectionClosed(SOCKADDR_BTH saddr);

private:
	map<UINT64, CRecevicedData*> m_mapReceived;
	pthread_mutex_t m_mutexlock;
	pthread_mutexattr_t m_mutexattr;
	HWND m_targethWnd;
	static void sendServerStatus(SOCKET socket, UINT32 status);
	static void decodeYUV420SP(int rgb[], char yuv420sp[], int width, int height);
	static int getRgbValueInRange(int value);
	static int getRgb(int y, int u, int v);
};

