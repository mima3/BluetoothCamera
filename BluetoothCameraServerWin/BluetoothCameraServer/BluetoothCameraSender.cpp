#include "stdafx.h"
#include "BluetoothCameraSender.h"
#include "BufferUtil.h"
#include "BluetoothCameraNetInterface.h"

/**
 * �T�[�o�[�X�e�[�^�X�̐ݒ�
 * @param[in] socket �ʐM�\�P�b�g
 * @param[in] status  0:���� 1:�_��
 */
void CBluetoothCameraSender::SetServerStatus(SOCKET socket, UINT32 status) {
	char buffer[12];
	size_t offset = 0;
	CBufferUtil::SetUINT32Data(&buffer[0], offset, DATA_CODE_SERVER_STATUS);
	CBufferUtil::SetUINT32Data(&buffer[0], offset, DATA_VERSION);
	CBufferUtil::SetUINT32Data(&buffer[0], offset, status);
	send(socket, buffer, 12, 0);
}

/**
 * �Ɩ��̐ݒ�
 * @param[in] socket �ʐM�\�P�b�g
 * @param[in] sts  0:���� 1:�_��
 */
void CBluetoothCameraSender::SetLightStatus(SOCKET socket,  UINT32 sts) {
	char buffer[12];
	size_t offset = 0;
	CBufferUtil::SetUINT32Data(&buffer[0], offset, DATA_CODE_SET_LIGHT_STATUS);
	CBufferUtil::SetUINT32Data(&buffer[0], offset, DATA_VERSION);
	CBufferUtil::SetUINT32Data(&buffer[0], offset, sts);

	send(socket, buffer, 12, 0);
}
