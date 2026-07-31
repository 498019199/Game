include(FetchContent)

# Pin the latest release instead of tracking master so builds stay reproducible.
FetchContent_Declare(
    dear_imgui_source
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(dear_imgui_source)

set(IMGUI_CORE_SOURCES
    ${dear_imgui_source_SOURCE_DIR}/imgui.cpp
    ${dear_imgui_source_SOURCE_DIR}/imgui_demo.cpp
    ${dear_imgui_source_SOURCE_DIR}/imgui_draw.cpp
    ${dear_imgui_source_SOURCE_DIR}/imgui_tables.cpp
    ${dear_imgui_source_SOURCE_DIR}/imgui_widgets.cpp)

if(ZENGINE_PLATFORM_WINDOWS)
    set(IMGUI_PLATFORM_SOURCES
        ${dear_imgui_source_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
        ${dear_imgui_source_SOURCE_DIR}/backends/imgui_impl_win32.cpp)
    set(IMGUI_DEMO_SOURCE
        ${dear_imgui_source_SOURCE_DIR}/examples/example_win32_directx11/main.cpp)
elseif(ZENGINE_PLATFORM_DARWIN)
    set(IMGUI_PLATFORM_SOURCES
        ${dear_imgui_source_SOURCE_DIR}/backends/imgui_impl_metal.mm
        ${dear_imgui_source_SOURCE_DIR}/backends/imgui_impl_osx.mm)
    set(IMGUI_DEMO_SOURCE
        ${dear_imgui_source_SOURCE_DIR}/examples/example_apple_metal/main.mm)
else()
    return()
endif()

add_library(imgui STATIC
    ${IMGUI_CORE_SOURCES}
    ${IMGUI_PLATFORM_SOURCES})

target_include_directories(imgui
    PUBLIC
        ${dear_imgui_source_SOURCE_DIR}
        ${dear_imgui_source_SOURCE_DIR}/backends)

if(ZENGINE_PLATFORM_WINDOWS)
    target_link_libraries(imgui PUBLIC d3d11)
elseif(ZENGINE_PLATFORM_DARWIN)
    set_target_properties(imgui PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
    target_compile_options(imgui PRIVATE -fobjc-arc)
    target_link_libraries(imgui
        PUBLIC
            "-framework AppKit"
            "-framework Metal"
            "-framework MetalKit"
            "-framework QuartzCore"
            "-framework GameController")
endif()

add_executable(imgui_demo ${IMGUI_DEMO_SOURCE})
target_link_libraries(imgui_demo PRIVATE imgui)

set_target_properties(imgui_demo PROPERTIES
    OUTPUT_NAME imgui_demo${ZENGINE_OUTPUT_SUFFIX}
    FOLDER "test"
)
set_target_properties(imgui PROPERTIES FOLDER "3rd/imgui")