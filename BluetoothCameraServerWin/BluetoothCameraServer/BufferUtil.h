#pragma once

/**
 * @class CBufferUtil
 * @brief �o�b�t�@����p�̃��[�e�B���e�B�N���X
 */
class CBufferUtil
{
public:
	static UINT64 GetUINT64Data(char* data, size_t &offset);
	static UINT32 GetUINT32Data(char* data, size_t &offset);

	static void SetUINT32Data(char* data, size_t &offset, UINT32 val);

};

