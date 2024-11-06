ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

if(MSVC)
    set(CMAKE_CXX_FLAGS "/Wall /W4 /EHsc /bigobj /Zc:strictStrings /Zc:rvalueCast /Gw")
    if(CMAKE_GENERATOR MATCHES "^Visual Studio")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    
    if(CMAKE_C_COMPILER_ID MATCHES Clang)
        message("Clang compiler")
        set(DEMOENGINE_COMPILER_NAME "clangcl")
        set(DEMOENGINE_COMPILER_CLANGCL TRUE)
    else()
        message("MSVC compiler")
        SET(DEMOENGINE_COMPILER_NAME "vc")
        SET(DEMOENGINE_COMPILER_MSVC TRUE)

        if(MSVC_VERSION GREATER_EQUAL 1930)
            set(DEMOENGINE_COMPILER_VERSION "143")
        elseif(MSVC_VERSION GREATER_EQUAL 1920)
            set(DEMOENGINE_COMPILER_VERSION "142")
        else()
            message(FATAL_ERROR "Unsupported compiler version. Please install VS2019 or up.")
        endif()

        if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION VERSION_GREATER_EQUAL 10.0.22000)
            set(WIN11_SDK TRUE)
        else()
            set(WIN11_SDK FALSE)
        endif()

		if((MSVC_VERSION GREATER_EQUAL 1929) AND ((NOT KLAYGE_PLATFORM_WINDOWS_STORE) OR WIN11_SDK))
            set(CMAKE_CXX_STANDARD 20)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
        else()
            set(CMAKE_CXX_STANDARD 17)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:throwingNew /permissive- /Zc:externConstexpr /Zc:__cplusplus")
        if((MSVC_VERSION GREATER_EQUAL 1925) AND WIN11_SDK)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor")
        endif()
        if(MSVC_VERSION GREATER_EQUAL 1935)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:templateScope")
        endif()
        if(DEMOENGINE_ARCH_NAME STREQUAL "x64")
            #SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Qspectre")
        endif()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /JMC")

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4061") # False positive on missing enum in switch case (This is a bad designed warning because there are "default"s to handle un-cased enums)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4255") # Allow func() to func(void) in some Windows SDK
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4365") # Ignore int to size_t
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4464") # Allow .. in include path
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4514") # Allow unused inline function
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4623") # Ignore implicitly deleted default constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4625") # Ignore implicitly deleted copy constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4626") # Ignore implicitly deleted copy operator=
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4668") # Undefined macro as 0
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4710") # Allow function with the inline mark not be inlined
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4711") # Allow function to be inlined without the inline mark
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4820") # Ignore padding
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5026") # Ignore implicitly deleted move constructor
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5027") # Ignore implicitly deleted move operator=
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5039") # Ignore passing a throwing function to C functions
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5045") # False positive on range check
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5204") # Ignore non trivial destructor in COM interfaces and ppl
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5219") # Ignore int to uint
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5220") # Ignore non trivial constructor in ppl
		if(MSVC_VERSION GREATER_EQUAL 1934)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5262") # Ignore implicit fall-through
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd5264") # Ignore unused const variable
		endif()

		SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
else()
    if(CMAKE_C_COMPILER_ID MATCHES Clang)
        set(DEMOENGINE_COMPILER_NAME "clang")
        set(DEMOENGINE_COMPILER_CLANG TRUE)
    elseif(MINGW)
        set(DEMOENGINE_COMPILER_NAME "mgw")
        set(DEMOENGINE_COMPILER_GCC TRUE)
    else()
        set(DEMOENGINE_COMPILER_NAME "gcc")
        set(DEMOENGINE_COMPILER_GCC TRUE)
    endif()
endif()

SET(DEMOENGINE_OUTPUT_SUFFIX _${DEMOENGINE_COMPILER_NAME}${DEMOENGINE_COMPILER_VERSION})

