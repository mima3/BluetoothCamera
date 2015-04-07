#include "stdafx.h"
#include "BufferUtil.h"

/**
 * �o�b�t�@����UINT64���擾����
 * @param [in, out] data   ����o�b�t�@
 * @param [in, out] offset �f�[�^�擾�̃I�t�Z�b�g
 */
UINT64 CBufferUtil::GetUINT64Data(char* data, size_t &offset) {
	UINT64 tmp64;
	memcpy(&tmp64, (data + offset), sizeof(UINT64));
	offset += sizeof(UINT64);
	return _byteswap_uint64(tmp64);
}

/**
 * �o�b�t�@����UINT32���擾����
 * @param [in, out] data   ����o�b�t�@
 * @param [in, out] offset �f�[�^�擾�̃I�t�Z�b�g
 */
UINT32 CBufferUtil::GetUINT32Data(char* data, size_t &offset) {
	UINT32 tmp32;
	memcpy(&tmp32, (data + offset), sizeof(UINT32));
	offset += sizeof(UINT32);
	return _byteswap_ulong(tmp32);
}

/**
* �o�b�t�@��UINT32��ݒ肷��
* @param [in, out] data   ����o�b�t�@
* @param [in, out] offset �f�[�^�擾�̃I�t�Z�b�g
* @param [in] val �ݒ�l
*/
void CBufferUtil::SetUINT32Data(char* data, size_t &offset, UINT32 val) {
	UINT32 tmp32 = _byteswap_ulong(val);
	memcpy((data + offset), &tmp32, sizeof(UINT32));
	offset += sizeof(UINT32);
}