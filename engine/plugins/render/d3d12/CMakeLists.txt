add_library(ZENGINE_Render_D3D12 ${ZENGINE_PREFERRED_LIB_TYPE}
    src/D3D12Fence.cpp
    src/D3D12Fence.h
    src/D3D12FrameBuffer.cpp
    src/D3D12FrameBuffer.h
    src/D3D12Gpu.cpp
    src/D3D12Gpu.h
    src/D3D12GraphicsBuffer.cpp
    src/D3D12GraphicsBuffer.h
    src/D3D12Interop.h
    src/D3D12Query.cpp
    src/D3D12Query.h
    src/D3D12RenderEngine.cpp
    src/D3D12RenderEngine.h
    src/D3D12RenderFactory.cpp
    src/D3D12RenderFactory.h
    src/D3D12RenderLayout.cpp
    src/D3D12RenderLayout.h
    src/D3D12RenderStateObject.cpp
    src/D3D12RenderStateObject.h
    src/D3D12RenderView.cpp
    src/D3D12RenderView.h
    src/D3D12RenderWindow.cpp
    src/D3D12RenderWindow.h
    src/D3D12ShaderObject.cpp
    src/D3D12ShaderObject.h
    src/D3D12Texture.cpp
    src/D3D12Texture.h
    src/D3D12Texture2D.cpp
    src/D3D12Texture3D.cpp
    src/D3D12TextureCube.cpp
    src/D3D12Util.cpp
    src/D3D12Util.h
    src/PixelFormats.inc
    src/VertexFormats.inc
)

target_precompile_headers(ZENGINE_Render_D3D12 PRIVATE <base/ZEngine.h>)
target_include_directories(ZENGINE_Render_D3D12 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(ZENGINE_Render_D3D12 PRIVATE ZENGINE_EXPORTS ZENGINE_BUILD_D3D12)
set_target_properties(ZENGINE_Render_D3D12 PROPERTIES
    OUTPUT_NAME ZENGINE_Render_D3D12${ZENGINE_OUTPUT_SUFFIX}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER "ZENGINE"
)
target_link_libraries(ZENGINE_Render_D3D12 PUBLIC ZENGINE_core PRIVATE ZENGINE_common d3d12 dxgi d3dcompiler)

# The editor and GPU probes import native D3D12 interop symbols before main().
# Keep the plugin copy for dynamic loading and stage the same DLL beside the apps.
if(ZENGINE_PREFERRED_LIB_TYPE STREQUAL "SHARED")
    add_custom_command(TARGET ZENGINE_Render_D3D12 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:ZENGINE_Render_D3D12>" "${ZENGINE_BIN_DIR}"
        COMMENT "Stage D3D12 runtime beside desktop applications"
        VERBATIM)
endif()

# dxcapi.h from Windows SDK (um)
if(DEFINED CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
	set(_ZENGINE_WINSDK_VER "${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")
else()
	set(_ZENGINE_WINSDK_VER "10.0.26100.0")
endif()
set(_ZENGINE_DXC_INCLUDE "$ENV{WindowsSdkDir}Include/${_ZENGINE_WINSDK_VER}/um")
if(NOT EXISTS "${_ZENGINE_DXC_INCLUDE}/dxcapi.h")
	set(_ZENGINE_DXC_INCLUDE "C:/Program Files (x86)/Windows Kits/10/Include/${_ZENGINE_WINSDK_VER}/um")
endif()
if(EXISTS "${_ZENGINE_DXC_INCLUDE}/dxcapi.h")
	target_include_directories(ZENGINE_Render_D3D12 PRIVATE "${_ZENGINE_DXC_INCLUDE}")
endif()

# Ship DXC runtime next to the plugin so D3D12/DXIL compile works without PATH setup.
set(_ZENGINE_DXC_REDIST "C:/Program Files (x86)/Windows Kits/10/Redist/D3D/x64")
if(EXISTS "${_ZENGINE_DXC_REDIST}/dxcompiler.dll")
	add_custom_command(TARGET ZENGINE_Render_D3D12 POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${_ZENGINE_DXC_REDIST}/dxcompiler.dll"
			"$<TARGET_FILE_DIR:ZENGINE_Render_D3D12>/dxcompiler.dll"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${_ZENGINE_DXC_REDIST}/dxil.dll"
			"$<TARGET_FILE_DIR:ZENGINE_Render_D3D12>/dxil.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_ZENGINE_DXC_REDIST}/dxcompiler.dll"
            "${_ZENGINE_DXC_REDIST}/dxil.dll"
            "${ZENGINE_BIN_DIR}"
		COMMENT "Copy DXC runtime next to ZENGINE_Render_D3D12"
        VERBATIM)
endif()
