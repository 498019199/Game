#include <common/ErrorHandling.h>
#include <format>
namespace CommonWorker
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
	void UnreachableInternal(std::string_view msg, std::string_view file, uint32_t line)
	{
		// if (!msg.empty())
		// {
		// 	LogError() << msg << std::endl;
		// }
		// LogError() << "UNREACHABLE executed";
		// if (!file.empty())
		// {
		// 	LogError() << " at " << file << ": " << line;
		// }
		// LogError() << "." << std::endl;

		std::unreachable();
	}
#endif
}