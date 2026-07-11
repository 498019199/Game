#pragma once

// Thin Tracy wrapper. Prefer this over including tracy headers directly.
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
