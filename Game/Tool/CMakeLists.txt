if(NOT ZENGINE_IS_DEV_PLATFORM)
    return()
endif()

add_executable(ZENGINE_model_convert
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set_target_properties(ZENGINE_model_convert PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON)

if(MSVC)
    target_compile_options(ZENGINE_model_convert PRIVATE /std:c++20)
endif()

target_include_directories(ZENGINE_model_convert
    PRIVATE
        ${ZENGINE_ROOT_HEADER}
        ${DEMOENGINE_DIR}/common/include
        ${DEMOENGINE_DIR}/core/core/include)

add_dependencies(ZENGINE_model_convert ZENGINE_DevHelps)

target_link_libraries(ZENGINE_model_convert
    PRIVATE
        ZENGINE_common
        ZENGINE_core
        ZENGINE_Render_D3D11
        ZENGINE_DevHelps
        d3dcompiler)

set_target_properties(ZENGINE_model_convert PROPERTIES
	FOLDER "Tool"
)
