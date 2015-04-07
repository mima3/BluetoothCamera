#pragma once

/**
 * @class CBufferUtil
 * @brief バッファ操作用のユーティリティクラス
 */
class CBufferUtil
{
public:
	static UINT64 GetUINT64Data(char* data, size_t &offset);
	static UINT32 GetUINT32Data(char* data, size_t &offset);

	static void SetUINT32Data(char* data, size_t &offset, UINT32 val);

};

