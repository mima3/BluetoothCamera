#include "stdafx.h"
#include "BluetoothCameraSender.h"
#include "BufferUtil.h"
#include "BluetoothCameraNetInterface.h"

void CBluetoothCameraSender::SetServerStatus(SOCKET socket, UINT32 status) {
	char buf[8];
	size_t offset = 0;
	CBufferUtil::SetUINT32Data(&buf[0], offset, DATA_CODE_SERVER_STATUS);
	CBufferUtil::SetUINT32Data(&buf[0], offset, status);
	send(socket, buf, 8, 0);
}

/**
* 証明の設定
* @param[in] socket 通信ソケット
* @param[in] sts  0:消灯 1:点灯
*/
void CBluetoothCameraSender::SetLightStatus(SOCKET socket,  UINT32 sts) {
	char buffer[8];
	size_t offset = 0;
	CBufferUtil::SetUINT32Data(&buffer[0], offset, DATA_CODE_SET_LIGHT_STATUS);
	CBufferUtil::SetUINT32Data(&buffer[0], offset, sts);

	send(socket, buffer, 8, 0);
}
