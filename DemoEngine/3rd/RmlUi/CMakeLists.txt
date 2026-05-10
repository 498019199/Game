# 上游源码克隆到 ${CMAKE_CURRENT_SOURCE_DIR}/RmlUi（见顶层 .gitignore）
UpdateExternalLib("RmlUi" "https://github.com/mikke89/RmlUi.git" "6.2")

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
set(RMLUI_COMPILER_OPTIONS OFF CACHE BOOL "" FORCE)
# none：不要求系统安装 FreeType，CMake 可直接通过（字体需自管位图引擎或后续再接 FreeType）。
# 若要用矢量字体：安装 FreeType 后改回 freetype，并保证 find_package 能找到，例如
#   vcpkg install freetype / 设置 Freetype_ROOT / 或自建 Freetype::Freetype ALIAS。
set(RMLUI_FONT_ENGINE "none" CACHE STRING "" FORCE)

add_subdirectory(RmlUi)

set_target_properties(rmlui PROPERTIES FOLDER "3rd/RmlUi")
set_target_properties(rmlui_core PROPERTIES FOLDER "3rd/RmlUi")
set_target_properties(rmlui_debugger PROPERTIES FOLDER "3rd/RmlUi")
