#include <common/ErrorHandling.h>
#include <format>
namespace RenderWorker
{
	std::string CombineFileLine(std::string_view file, uint32_t line)
	{
		return std::format("{}: {}", std::move(file), line);
	}
}