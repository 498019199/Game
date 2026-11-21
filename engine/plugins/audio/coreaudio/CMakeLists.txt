add_library(ZENGINE_AudioEngine_CoreAudio ${ZENGINE_PREFERRED_LIB_TYPE} CoreAudio.cpp)
target_precompile_headers(ZENGINE_AudioEngine_CoreAudio PRIVATE <base/ZEngine.h>)
target_link_libraries(ZENGINE_AudioEngine_CoreAudio PRIVATE ZENGINE_core "-framework AudioToolbox")
set_target_properties(ZENGINE_AudioEngine_CoreAudio PROPERTIES
    OUTPUT_NAME ZENGINE_AudioEngine_CoreAudio${ZENGINE_OUTPUT_SUFFIX}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    FOLDER "ZENGINE")
