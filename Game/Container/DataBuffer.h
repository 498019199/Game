// ÄÚ´æ¹ÜÀí
#ifndef _STX_DataBuffer_BUFFER_
#define _STX_DataBuffer_BUFFER_
#pragma once
#include "../Core/predefine.h"
#include "var_type.h"

#include <boost/assert.hpp>
class DataBuffer
{
public:
	DataBuffer();

	DataBuffer(const DataBuffer& other);

	DataBuffer(DataBuffer&& other);

	~DataBuffer();

	DataBuffer& operator= (const DataBuffer& other);

	DataBuffer& operator= (DataBuffer&& other);

	uint8_t* GetConstantBuffer() const;

	uint32_t GetSize() const;

	void Copy(const uint8_t* bytes, const uint32_t size);

	void Create(const uint32_t size);

	void FastSet(uint8_t* bytes, const uint32_t size);

	void Clear();

	bool IsNull() const;
private:
	void DataMove(DataBuffer& other);
private:
	uint8_t *m_pDataBuffer;
	uint32_t m_nDataLen;
};
#endif//_STX_DataBuffer_BUFFER_