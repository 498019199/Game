#pragma once

#include <common/CpuProfiler.h>

// Thin Tracy wrapper + local CpuProfiler zones for the Editor panel.
#if defined(ZENGINE_ENABLE_TRACY)
#	include <tracy/Tracy.hpp>
#else
#	ifndef ZoneScoped
#		define ZoneScoped
#	endif
#	ifndef ZoneScopedN
#		define ZoneScopedN(name)
#	endif
#	ifndef ZoneNamedN
#		define ZoneNamedN(varname, name, active)
#	endif
#	ifndef FrameMark
#		define FrameMark
#	endif
#	ifndef TracyPlot
#		define TracyPlot(name, val)
#	endif
#	ifndef TracyMessageL
#		define TracyMessageL(text)
#	endif
#endif

#define ZENGINE_ZONE_CONCAT_INNER(a, b) a##b
#define ZENGINE_ZONE_CONCAT(a, b) ZENGINE_ZONE_CONCAT_INNER(a, b)

/// Local timing for ImGui profiler + Tracy zone when enabled.
#define ZENGINE_ZONE(name) \
	ZoneScopedN(name); \
	::CommonWorker::ScopedCpuZone ZENGINE_ZONE_CONCAT(_zengine_cpu_zone_, __LINE__)(name)
