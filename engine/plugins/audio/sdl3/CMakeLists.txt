add_library(ZENGINE_AudioEngine_SDL3 ${ZENGINE_PREFERRED_LIB_TYPE} SDL3Audio.cpp)
target_precompile_headers(ZENGINE_AudioEngine_SDL3 PRIVATE <base/ZEngine.h>)
target_link_libraries(ZENGINE_AudioEngine_SDL3 PRIVATE ZENGINE_core SDL3::SDL3)
set_target_properties(ZENGINE_AudioEngine_SDL3 PROPERTIES
    OUTPUT_NAME ZENGINE_AudioEngine_SDL3${ZENGINE_OUTPUT_SUFFIX}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER "ZENGINE")
