# Tracy Profiler — https://github.com/wolfpld/tracy
# Pinned to v0.13.1. Enable via -DZENGINE_ENABLE_TRACY=ON at the engine root.
UpdateExternalLib("tracy" "https://github.com/wolfpld/tracy.git" "05cceee0df3b8d7c6fa87e9638af311dbabc63cb")

# Shared client so core / render / game / editor share one Tracy instance.
set(TRACY_STATIC OFF CACHE BOOL "" FORCE)

if(ZENGINE_ENABLE_TRACY)
	set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
else()
	set(TRACY_ENABLE OFF CACHE BOOL "" FORCE)
endif()

if(ZENGINE_TRACY_ON_DEMAND)
	set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
else()
	set(TRACY_ON_DEMAND OFF CACHE BOOL "" FORCE)
endif()

add_subdirectory(tracy EXCLUDE_FROM_ALL)

if(TARGET TracyClient)
	set_target_properties(TracyClient PROPERTIES FOLDER "3rd/tracy")
endif()
