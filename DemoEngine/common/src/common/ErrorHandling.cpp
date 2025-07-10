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
}