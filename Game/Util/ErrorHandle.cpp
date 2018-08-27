#include "../Util/ErrorHandle.h"

#include <string>
#include <system_error>
std::string CombineFileLine(std::string_view file, int line)
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
