#include <common/ErrorHandling.h>

#include <ostream>
#include <string>
#include <string_view>
#include <system_error>

#include <common/Log.h>

namespace CommonWorker
{
	std::string CombineFileLine(std::string_view file, uint32_t line)
	{
		return std::string(file) + ": " + std::to_string(line);
	}

	void Verify(bool x)
	{
		if (!x)
		{
			TERRC(std::errc::function_not_supported);
		}
	}

	void UnreachableInternal(std::string_view msg, std::string_view file, uint32_t line)
	{
		(void)msg;
		(void)file;
		(void)line;
		if (!msg.empty())
		{
		// 	LogError() << msg << std::endl;
		}
		// LogError() << "UNREACHABLE executed";
		if (!file.empty())
		{
		// 	LogError() << " at " << file << ": " << line;
		}
		// LogError() << "." << std::endl;

		std::unreachable();
	}
}
