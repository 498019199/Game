#include <common/common.h>
#include <common/ErrorHandling.h>

#include <format>
#include <utility>

namespace KlayGE
{
	std::string CombineFileLine(std::string_view file, uint32_t line)
	{
		return std::format("{}: {}", std::move(file), line);
	}

	void Verify(bool x)
	{
		if (!x)
		{
			TERRC(std::errc::function_not_supported);
		}
	}

#if defined(_DEBUG)
	void KFLUnreachableInternal(std::string_view msg, std::string_view file, uint32_t line)
	{
		if (!msg.empty())
		{
			LOGER_ERROR() << msg << std::endl;
		}
		LOGER_ERROR() << "UNREACHABLE executed";
		if (!file.empty())
		{
			LOGER_ERROR() << " at " << file << ": " << line;
		}
		LOGER_ERROR() << "." << std::endl;

#ifdef __cpp_lib_unreachable
		std::unreachable();
#endif//__cpp_lib_unreachable
	}
#endif

}