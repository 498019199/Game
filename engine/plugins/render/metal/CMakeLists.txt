add_library(ZENGINE_Render_Metal ${ZENGINE_PREFERRED_LIB_TYPE}
    src/MetalFence.cpp
    src/MetalFence.h
    src/MetalFrameBuffer.cpp
    src/MetalFrameBuffer.h
    src/MetalGpu.h
    src/MetalGpu.mm
    src/MetalGraphicsBuffer.cpp
    src/MetalGraphicsBuffer.h
    src/MetalQuery.cpp
    src/MetalQuery.h
    src/MetalRenderEngine.cpp
    src/MetalRenderEngine.h
    src/MetalRenderFactory.cpp
    src/MetalRenderFactory.h
    src/MetalRenderLayout.cpp
    src/MetalRenderLayout.h
    src/MetalRenderStateObject.cpp
    src/MetalRenderStateObject.h
    src/MetalRenderView.cpp
    src/MetalRenderView.h
    src/MetalRenderWindow.cpp
    src/MetalRenderWindow.h
    src/MetalShaderCompiler.cpp
    src/MetalShaderCompiler.h
    src/MetalShaderObject.cpp
    src/MetalShaderObject.h
    src/MetalTexture.cpp
    src/MetalTexture.h
    src/MetalTexture2D.cpp
    src/MetalTexture3D.cpp
    src/MetalTextureCube.cpp
    src/MetalUtil.cpp
    src/MetalUtil.h
    src/PixelFormats.inc
    src/VertexFormats.inc
)

target_precompile_headers(ZENGINE_Render_Metal PRIVATE <base/ZEngine.h>)
target_include_directories(ZENGINE_Render_Metal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(ZENGINE_Render_Metal PRIVATE ZENGINE_EXPORTS ZENGINE_BUILD_METAL)
set_target_properties(ZENGINE_Render_Metal PROPERTIES
    OUTPUT_NAME ZENGINE_Render_Metal${ZENGINE_OUTPUT_SUFFIX}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER "ZENGINE"
)
set_source_files_properties(src/MetalGpu.mm PROPERTIES SKIP_PRECOMPILE_HEADERS ON COMPILE_FLAGS "-fobjc-arc")
target_compile_definitions(ZENGINE_Render_Metal PRIVATE ENABLE_HLSL)
target_link_libraries(ZENGINE_Render_Metal
    PUBLIC ZENGINE_core
    PRIVATE ZENGINE_common "-framework AppKit" "-framework Metal" "-framework QuartzCore"
        glslang::glslang glslang::SPIRV glslang::glslang-default-resource-limits spirv-cross-msl
)
