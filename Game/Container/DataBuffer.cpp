#include "DataBuffer.h"
#include "momery.h"
#include <memory>

DataBuffer::DataBuffer()
	:m_pDataBuffer(nullptr), m_nDataLen(0)
{

}

DataBuffer::DataBuffer(const DataBuffer& other)
	: m_pDataBuffer(nullptr), m_nDataLen(0)
{
	Copy(other.m_pDataBuffer, other.m_nDataLen);
}

DataBuffer::DataBuffer(DataBuffer&& other)
	: m_pDataBuffer(nullptr), m_nDataLen(0)
{
	DataMove(other);
}

DataBuffer::~DataBuffer()
{
	Clear();
}

uint8_t* DataBuffer::GetConstantBuffer() const
{
	return m_pDataBuffer;
}

uint32_t DataBuffer::GetSize() const
{
	return m_nDataLen;
}

void DataBuffer::Copy(const uint8_t* bytes, const uint32_t size)
{
	Clear();

	if (size > 0)
	{
		m_nDataLen = size;
		m_pDataBuffer = NEW uint8_t[(sizeof(uint8_t) * m_nDataLen)];
		memcpy(m_pDataBuffer, bytes, m_nDataLen);
	}
}

void DataBuffer::Create(const uint32_t size)
{
	Clear();
	m_nDataLen = size;
	m_pDataBuffer = NEW uint8_t[(sizeof(uint8_t) * m_nDataLen)];
}

void DataBuffer::FastSet(uint8_t* bytes, const uint32_t size)
{
	m_nDataLen = size;
	m_pDataBuffer = bytes;
}

void DataBuffer::Clear()
{
	delete[] m_pDataBuffer;

	m_nDataLen = 0;
	m_pDataBuffer = nullptr;
}

bool DataBuffer::IsNull() const
{
	return (m_pDataBuffer == nullptr || m_nDataLen == 0);
}

void DataBuffer::DataMove(DataBuffer& other)
{
	m_pDataBuffer = other.m_pDataBuffer;
	m_nDataLen = other.m_nDataLen;

	other.m_pDataBuffer = nullptr;
	other.m_nDataLen = 0;
}

DataBuffer& DataBuffer::operator=(DataBuffer&& other)
{
	DataMove(other);
	return *this;
}

DataBuffer& DataBuffer::operator=(const DataBuffer& other)
{
	Copy(other.m_pDataBuffer, other.m_nDataLen);
	return *this;
}
