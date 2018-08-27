#ifndef _STX_PACKAGE_HPP
#define _STX_PACKAGE_HPP
#pragma once
#include "../Core/predefine.h"
#include "../Container/C++17/string_view.h"

struct IInArchive;
class  Package
{
public:
	explicit Package(ResIdentifierPtr const & archive_is);
	Package(ResIdentifierPtr const & archive_is, std::string_view password);

	bool Locate(std::string_view extract_file_path);
	ResIdentifierPtr Extract(std::string_view extract_file_path, std::string_view res_name);

	ResIdentifier* ArchiveStream() const
	{
		return archive_is_.get();
	}

private:
	uint32_t Find(std::string_view extract_file_path);

private:
	ResIdentifierPtr archive_is_;

	std::shared_ptr<IInArchive> archive_;
	std::string password_;

	uint32_t num_items_;
};
#endif//_STX_PACKAGE_HPP