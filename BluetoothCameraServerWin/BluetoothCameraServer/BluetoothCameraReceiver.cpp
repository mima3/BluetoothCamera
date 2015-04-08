#include "stdafx.h"
#include "BluetoothCameraReceiver.h"
#include <iostream>
#include <fstream>
#include "BluetoothAppDefine.h"
#include "AutoLock.h"
#include "BufferUtil.h"
#include "BluetoothCameraNetInterface.h"
#include "BluetoothCameraSender.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static const int YUV_ELEMENT_MASK = 0xff;
static const int RED_ELEMENT_MASK = 0xff0000;
static const int GREEN_ELEMENT_MASK = 0xff00;
static const int BLUE_ELEMENT_MASK = 0xff;
static const int RED_ELEMENT_SHIFT = 6;
static const int GREEN_ELEMENT_SHIFT = 2;
static const int BLUE_ELEMENT_SHIFT = 10;
static const int ALPHA_ELEMENT_FULL = 0xff000000;

CRecevicedData::CRecevicedData(UINT64 addr) :
	m_Addr(addr),
	m_status(kStatus_None),
	m_BufferSize(0),
	m_currentPos(0),
	m_buff(NULL),
	m_width(0),
	m_height(0)
{

}

CRecevicedData::~CRecevicedData() {
	if (this->m_buff) {
		delete this->m_buff;
	}
}

/**
 * コンストラクタ
 * @param[in] HWND   targethWnd メッセージを受信するWindowsのハンドラ
 */
CBluetoothCameraReceiver::CBluetoothCameraReceiver(HWND targethWnd) :
	m_targethWnd(targethWnd),
	m_mutexlock(PTHREAD_MUTEX_INITIALIZER)
{
	pthread_mutexattr_init(&this->m_mutexattr);
	pthread_mutexattr_settype(&this->m_mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&this->m_mutexlock, &this->m_mutexattr);
}


CBluetoothCameraReceiver::~CBluetoothCameraReceiver()
{
	pthread_mutexattr_destroy(&this->m_mutexattr);
	pthread_mutex_destroy(&this->m_mutexlock);
	for (map<UINT64, CRecevicedData*>::iterator itr = m_mapReceived.begin();
		itr != m_mapReceived.end(); ++itr) {
		delete itr->second;
	}
	m_mapReceived.clear();
}

/**
 * エラーコールバック
 * @param [in] BluetoothServerError エラー情報
 */
void CBluetoothCameraReceiver::OnErrorCallback(const BluetoothServerError& error) {
	static WCHAR msg[1024];
	wsprintf(msg, L"(0x%x) %s\n", error.errorcode, error.message);
	::PostMessage(this->m_targethWnd, WM_BLUETOOTH_ERROR, (WPARAM)&msg, NULL);
}


/**
 * 接続開始のコールバック
 */
bool CBluetoothCameraReceiver::OnConnectedCallback(SOCKET socket, SOCKADDR_BTH saddr) {
	CAutoLock lock(&this->m_mutexlock);

	UINT64 addr = GET_SAP(saddr.btAddr);
	::SendMessage(this->m_targethWnd, WM_BLUETOOTH_CONNECTED, (WPARAM)&addr, NULL);

	CRecevicedData* pData = new CRecevicedData(addr);
	this->m_mapReceived[addr] = pData;

	CBluetoothCameraSender::SetServerStatus(socket, 1);

	return true;
}




void CBluetoothCameraReceiver::OnReceivedCallback(SOCKET socket, SOCKADDR_BTH saddr, char* data, int recvSize) {
	UINT64 addr = GET_SAP(saddr.btAddr);
	CAutoLock lock(&this->m_mutexlock);
	CRecevicedData *pRcvData = this->m_mapReceived[addr];

	//
	if (pRcvData->m_status == CRecevicedData::kStatus_None) {
		// 初回の受信
		UINT32 tmp32;
		size_t offset = 0;
		tmp32 = CBufferUtil::GetUINT32Data(data, offset);
		if (tmp32 != DATA_CODE_PICTURE) {
			// 指定のコード以外の場合、予期せぬデータなので無視。
			return;
		}
		// サーバの受信を無効にする
		CBluetoothCameraSender::SetServerStatus(socket, 0);

		pRcvData->m_BufferSize = (size_t)CBufferUtil::GetUINT64Data(data, offset);
		pRcvData->m_width = CBufferUtil::GetUINT32Data(data, offset);
		pRcvData->m_height = CBufferUtil::GetUINT32Data(data, offset);
		if (pRcvData->m_buff) {
			delete pRcvData;
		}
		pRcvData->m_buff = new char[pRcvData->m_BufferSize];
		pRcvData->m_currentPos = 0;
		pRcvData->m_status = CRecevicedData::kStatus_Receiving;
		recvSize -= (offset);
		data += (offset);
	}

	if (pRcvData->m_status == CRecevicedData::kStatus_Receiving) {
		UINT64 cpSize = recvSize;
		if (cpSize > pRcvData->m_BufferSize - pRcvData->m_currentPos) {
			cpSize = pRcvData->m_BufferSize - pRcvData->m_currentPos;
		}
		memcpy((pRcvData->m_buff + pRcvData->m_currentPos), data, (size_t)cpSize);
		pRcvData->m_currentPos += recvSize;
		if (pRcvData->m_currentPos >= pRcvData->m_BufferSize) {
			/*
			ofstream fout;
			fout.open("test.jpg", ios::out | ios::binary | ios::trunc);
			if (fout) {
				fout.write(this->m_buff, this->m_BufferSize);
				fout.close();
			}
			*/
			// 全て受信
			int* rgb;
			BITMAPINFO* lpBitmap = new BITMAPINFO;
			ZeroMemory(lpBitmap, sizeof(BITMAPINFO));
			lpBitmap->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			lpBitmap->bmiHeader.biWidth = pRcvData->m_width;
			lpBitmap->bmiHeader.biHeight = pRcvData->m_height;
			lpBitmap->bmiHeader.biPlanes = 1;
			lpBitmap->bmiHeader.biBitCount = 32;
			lpBitmap->bmiHeader.biCompression = BI_RGB;
			HBITMAP hBmp = CreateDIBSection(NULL, lpBitmap, DIB_RGB_COLORS, (void**)&rgb, NULL, 0);
			CBluetoothCameraReceiver::decodeYUV420SP(rgb, pRcvData->m_buff, pRcvData->m_width, pRcvData->m_height);
			delete lpBitmap;

			CameraImageData* pData = new CameraImageData;
			pData->bufferSize = pRcvData->m_BufferSize;
			pData->hBitmap = hBmp;
			pData->deviceId = addr;
			::PostMessage(this->m_targethWnd, WM_BLUETOOTH_RECEIVED, (WPARAM)pData, NULL);

			delete pRcvData->m_buff;
			pRcvData->m_buff = NULL;
			pRcvData->m_status = CRecevicedData::kStatus_None;
			pRcvData->m_BufferSize = 0;
			pRcvData->m_currentPos = 0;
			pRcvData->m_width = 0;
			pRcvData->m_height = 0;

			// サーバの受信を有効にする
			CBluetoothCameraSender::SetServerStatus(socket, 1);
		}
	}
}

/**
 * 接続の切断イベント
 * @param [in] SOCKADDR_BTH saddr 切断された接続先
 */
void CBluetoothCameraReceiver::OnConnectionClosed(SOCKADDR_BTH saddr) {
	UINT64 addr = GET_SAP(saddr.btAddr);

	CAutoLock lock(&this->m_mutexlock);
	if (this->m_mapReceived[addr]) {
		delete this->m_mapReceived[addr];
		this->m_mapReceived.erase(addr);
	}
	::SendMessage(this->m_targethWnd, WM_BLUETOOTH_CLOSED, (WPARAM)&addr, NULL);
}





/**
 * AndroidのカメラPreviewのYUVをRGBに変換する
 * http://qiita.com/GeneralD/items/68142abb852c392db236
 * @param [out] int[] rgb       変換されたRGB
 * @param [in]  char[] yuv420sp YUVのバッファ
 * @param [in]  int width       幅
 * @param [in]  int height      高さ
 */
void CBluetoothCameraReceiver::decodeYUV420SP(int rgb[], char yuv420sp[], int width, int height) {
	const int frameSize = width * height;

	for (int j = 0, yp = 0; j < height; j++) {
		int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
		for (int i = 0; i < width; i++, yp++) {
			int y = (0xff & ((int)yuv420sp[yp])) - 16;
			if (y < 0) y = 0;
			if ((i & 1) == 0) {
				v = (0xff & yuv420sp[uvp++]) - 128;
				u = (0xff & yuv420sp[uvp++]) - 128;
			}

			int y1192 = 1192 * y;
			int r = (y1192 + 1634 * v);
			int g = (y1192 - 833 * v - 400 * u);
			int b = (y1192 + 2066 * u);

			if (r < 0) r = 0; else if (r > 262143) r = 262143;
			if (g < 0) g = 0; else if (g > 262143) g = 262143;
			if (b < 0) b = 0; else if (b > 262143) b = 262143;

			rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000) | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
		}
	}
}

int CBluetoothCameraReceiver::getRgbValueInRange(int value) {
	return max(0x0, min(value, 0x3ffff));
}

int CBluetoothCameraReceiver::getRgb(int y, int u, int v) {

	int y0x4a8 = 0x4a8 * y;
	int r = getRgbValueInRange(y0x4a8 + 0x662 * v);
	int g = getRgbValueInRange(y0x4a8 - 0x341 * v - 0x190 * u);
	int b = getRgbValueInRange(y0x4a8 + 0x812 * u);
	return ALPHA_ELEMENT_FULL | ((r << RED_ELEMENT_SHIFT) & RED_ELEMENT_MASK)
		| ((g >> GREEN_ELEMENT_SHIFT) & GREEN_ELEMENT_MASK) | ((b >> BLUE_ELEMENT_SHIFT) & BLUE_ELEMENT_MASK);
}
