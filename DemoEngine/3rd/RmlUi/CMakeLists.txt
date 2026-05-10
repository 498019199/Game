# 上游源码克隆到 ${CMAKE_CURRENT_SOURCE_DIR}/RmlUi（见顶层 .gitignore）
UpdateExternalLib("RmlUi" "https://github.com/mikke89/RmlUi.git" "6.2")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_COMPILER_OPTIONS OFF CACHE BOOL "" FORCE)
# 使用工程内 3rd/freetype（先于本目录加入），供 RmlUi 矢量字体；满足 RmlUi 对 Freetype::Freetype 目标的要求。
if(NOT TARGET Freetype::Freetype AND TARGET freetype)
	add_library(Freetype::Freetype ALIAS freetype)
endif()
set(CMAKE_DISABLE_FIND_PACKAGE_Freetype TRUE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)

add_subdirectory(RmlUi)

unset(CMAKE_DISABLE_FIND_PACKAGE_Freetype)

set_target_properties(rmlui PROPERTIES FOLDER "3rd/RmlUi")
set_target_properties(rmlui_core PROPERTIES FOLDER "3rd/RmlUi")
set_target_properties(rmlui_debugger PROPERTIES FOLDER "3rd/RmlUi")
