#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

//---------------------------------------------------------------------------------------
// Simple d3d error checker for book demos.
//---------------------------------------------------------------------------------------
// Throw error message
namespace RenderWorker
{
	std::string CombineFileLine(std::string_view file, uint32_t line);
}

// Throw error code
#define TEC(x) throw std::system_error(x, RenderWorker::CombineFileLine(__FILE__, __LINE__))

// Throw error message
#define TMSG(msg) throw std::runtime_error(msg)

// Throw if failed (error code)
#define TIFEC(x)   \
	{              \
		if (x)     \
		{          \
			TEC(x) \
		}          \
	}

// Throw if failed (errc)
#define TERRC(x) TEC(std::make_error_code(x))

// Throw if failed (errc)
#define TIFERRC(x) TIFEC(std::make_error_code(x))

// Throw if failed (HRESULT)
#define TIFHR(hr)                                              		 \
	{                                                          		 \
		if ((hr) < 0)                                          		 \
		{                                                      		 \
			TMSG(RenderWorker::CombineFileLine(__FILE__, __LINE__)); \
		}                                                      		 \
	}