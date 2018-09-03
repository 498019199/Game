#ifndef _RESIDENTIFIER_HPP_
#define _RESIDENTIFIER_HPP_
#pragma once
#include "../Core/predefine.h"
#include <istream>
#include <string>
#include "../Container/C++17/string_view.h"
class ResIdentifier
{
public:
	ResIdentifier(std::string_view name, uint64_t timestamp,
		std::shared_ptr<std::istream> const & is)
		: ResIdentifier(name, timestamp, is, std::shared_ptr<std::streambuf>())
	{
	}
	ResIdentifier(std::string_view name, uint64_t timestamp,
		std::shared_ptr<std::istream> const & is, std::shared_ptr<std::streambuf> const & streambuf)
		: m_strNameRes(name), m_nTimestamp(timestamp), m_IStream(is), m_Streambuf(streambuf)
	{
	}

	void ResName(std::string_view name)
	{
		m_strNameRes = std::string(name);
	}
	std::string const & ResName() const
	{
		return m_strNameRes;
	}

	void Timestamp(uint64_t ts)
	{
		m_nTimestamp = ts;
	}
	uint64_t Timestamp() const
	{
		return m_nTimestamp;
	}

	void read(void* p, size_t size)
	{
		m_IStream->read(static_cast<char*>(p), static_cast<std::streamsize>(size));
	}
	bool empty() const
	{
		return gcount() == 0;
	}
	int64_t gcount() const
	{
		return static_cast<int64_t>(m_IStream->gcount());
	}

	void seekg(int64_t offset, std::ios_base::seekdir way)
	{
		m_IStream->seekg(static_cast<std::istream::off_type>(offset), way);
	}

	int64_t tellg()
	{
		return static_cast<int64_t>(m_IStream->tellg());
	}

	void clear()
	{
		m_IStream->clear();
	}

	operator bool() const
	{
		return !m_IStream->fail();
	}

	bool operator!() const
	{
		return m_IStream->operator!();
	}

	std::istream& input_stream()
	{
		return *m_IStream;
	}

private:
	std::string m_strNameRes;
	uint64_t m_nTimestamp;
	std::shared_ptr<std::istream> m_IStream;
	std::shared_ptr<std::streambuf> m_Streambuf;
};
#endif // _RESIDENTIFIER_HPP_