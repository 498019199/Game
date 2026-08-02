if(NOT ZENGINE_IS_DEV_PLATFORM)
    return()
endif()

# --- model convert ---
add_executable(ZENGINE_model_convert
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set_target_properties(ZENGINE_model_convert PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "Tool"
)

if(MSVC)
    target_compile_options(ZENGINE_model_convert PRIVATE /std:c++20)
endif()

target_include_directories(ZENGINE_model_convert
    PRIVATE
        ${ZENGINE_ROOT_HEADER}
        ${DEMOENGINE_DIR}/common/include
        ${DEMOENGINE_DIR}/core/core/include)

add_dependencies(ZENGINE_model_convert ZENGINE_DevHelps)

if(ZENGINE_PLATFORM_WINDOWS)
	set(ZENGINE_MODEL_CONVERT_RENDER_LIBS
		ZENGINE_Render_D3D11
		d3dcompiler)
else()
	set(ZENGINE_MODEL_CONVERT_RENDER_LIBS
		ZENGINE_Render_SDL3)
endif()

target_link_libraries(ZENGINE_model_convert
    PRIVATE
        ZENGINE_common
        ZENGINE_core
        ZENGINE_DevHelps
        ${ZENGINE_MODEL_CONVERT_RENDER_LIBS})

# --- shader bake (platform-specific .kfx) ---
add_executable(ZENGINE_shader_bake
    ${CMAKE_CURRENT_SOURCE_DIR}/shader_bake_main.cpp)

set_target_properties(ZENGINE_shader_bake PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    FOLDER "Tool"
)

if(MSVC)
    target_compile_options(ZENGINE_shader_bake PRIVATE /std:c++20)
endif()

target_include_directories(ZENGINE_shader_bake
    PRIVATE
        ${ZENGINE_ROOT_HEADER}
        ${DEMOENGINE_DIR}/common/include
        ${DEMOENGINE_DIR}/core/core/include)

add_dependencies(ZENGINE_shader_bake ZENGINE_DevHelps)

if(ZENGINE_PLATFORM_WINDOWS)
	# Runtime loads render DLLs by name; link D3D11 for create path, depend on SDL3 so both plugins build.
	add_dependencies(ZENGINE_shader_bake ZENGINE_Render_D3D11 ZENGINE_Render_SDL3)
	target_link_libraries(ZENGINE_shader_bake
		PRIVATE
			ZENGINE_common
			ZENGINE_core
			ZENGINE_DevHelps
			ZENGINE_Render_D3D11
			d3dcompiler)
else()
	target_link_libraries(ZENGINE_shader_bake
		PRIVATE
			ZENGINE_common
			ZENGINE_core
			ZENGINE_DevHelps
			ZENGINE_Render_SDL3)
endif()
