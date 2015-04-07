#pragma once
#include "BluetoothServer.h"

/**
 * @struct CameraImageData
 * @breif �J�����摜�f�[�^
 */
struct CameraImageData {
	UINT64 deviceId;	//! �f�o�C�X��GUID
	size_t bufferSize;	//! �o�b�t�@�̃T�C�Y
	HBITMAP hBitmap;	//! �r�b�g�}�b�v�̃n���h��
};

/**
* @class CRecevicedData
* @brief ��M�f�[�^�̃N���X
*/
class CRecevicedData {
public:
	CRecevicedData(UINT64 addr);
	~CRecevicedData();

	enum eStatus {
		kStatus_None = 0,		// ����M
		kStatus_Receiving = 1	// ���݃f�[�^����M��
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
 * @brief Bluetooth����̎�M���������ĉ摜�Ƃ��ĕϊ�����
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

