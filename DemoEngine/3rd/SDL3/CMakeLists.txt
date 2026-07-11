# SDL3 — cross-platform windowing + GPU API (Vulkan / Metal)
# Windows builds use Direct3D 11 and do not include this dependency.
UpdateExternalLib("SDL" "https://github.com/libsdl-org/SDL.git" "release-3.4.10")

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL ON CACHE BOOL "" FORCE)

if(ZENGINE_PLATFORM_DARWIN OR ZENGINE_PLATFORM_IOS)
	set(SDL_RENDER_METAL ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(SDL EXCLUDE_FROM_ALL)

if(TARGET SDL3-shared)
	set_target_properties(SDL3-shared PROPERTIES FOLDER "3rd/SDL3")
endif()
if(TARGET SDL3-static)
	set_target_properties(SDL3-static PROPERTIES FOLDER "3rd/SDL3")
endif()
if(TARGET SDL_uclibc)
	set_target_properties(SDL_uclibc PROPERTIES FOLDER "3rd/SDL3")
endif()
if(TARGET SDL3_test)
	set_target_properties(SDL3_test PROPERTIES FOLDER "3rd/SDL3")
endif()

# Alias used by render plugin / core
if(NOT TARGET SDL3::SDL3)
	if(TARGET SDL3-shared)
		add_library(SDL3::SDL3 ALIAS SDL3-shared)
	elseif(TARGET SDL3-static)
		add_library(SDL3::SDL3 ALIAS SDL3-static)
	endif()
endif()
