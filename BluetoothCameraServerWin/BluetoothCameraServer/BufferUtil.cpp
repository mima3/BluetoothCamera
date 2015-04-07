#include "stdafx.h"
#include "BufferUtil.h"

/**
 * バッファからUINT64を取得する
 * @param [in, out] data   操作バッファ
 * @param [in, out] offset データ取得のオフセット
 */
UINT64 CBufferUtil::GetUINT64Data(char* data, size_t &offset) {
	UINT64 tmp64;
	memcpy(&tmp64, (data + offset), sizeof(UINT64));
	offset += sizeof(UINT64);
	return _byteswap_uint64(tmp64);
}

/**
 * バッファからUINT32を取得する
 * @param [in, out] data   操作バッファ
 * @param [in, out] offset データ取得のオフセット
 */
UINT32 CBufferUtil::GetUINT32Data(char* data, size_t &offset) {
	UINT32 tmp32;
	memcpy(&tmp32, (data + offset), sizeof(UINT32));
	offset += sizeof(UINT32);
	return _byteswap_ulong(tmp32);
}

/**
* バッファにUINT32を設定する
* @param [in, out] data   操作バッファ
* @param [in, out] offset データ取得のオフセット
* @param [in] val 設定値
*/
void CBufferUtil::SetUINT32Data(char* data, size_t &offset, UINT32 val) {
	UINT32 tmp32 = _byteswap_ulong(val);
	memcpy((data + offset), &tmp32, sizeof(UINT32));
	offset += sizeof(UINT32);
}