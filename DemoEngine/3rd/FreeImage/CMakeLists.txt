SET(FREEIMAGE_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/FreeImage")

UpdateExternalLib("FreeImage" "https://github.com/gongminmin/FreeImage.git" "7c453922d9442cd1f03a751d1f9273236405f9b7" "master" "3.18.0" need_patch)
if(TRUE)
	foreach(patch "0001-Fix-compiling-problems-on-VS2017-15.3-with-std-c-17" "0002-Turn-off-SSE2-for-arm-and-arm64-on-msvc"
		"0003-Fix-some-compiling-problems-on-MinGW-7.x" "0004-Fix-beta-for-GCC8" "0005-Fix-missing-__mingw_uuidof_s-problem-on-gcc-8.3"
		"0006-Fix-compiling-problems-on-VS2019-Update-4" "0007-Fix-for-clang-cl" "0008-Remove-register-to-fix-errors-in-C-17-or-up"
		"0009-Fix-compilation-problems-on-clang-16")
		ApplyPatch("FreeImage" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

IF(ZENGINE_COMPILER_MSVC)
	STRING(REPLACE "/WX" "/WX:NO" CMAKE_STATIC_LINKER_FLAGS ${CMAKE_STATIC_LINKER_FLAGS})
ELSEIF(ZENGINE_COMPILER_CLANG)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
elseif(ZENGINE_COMPILER_CLANGCL)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()
FOREACH(flag_var
		CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
	STRING(REPLACE "${NO_RTTI_FLAG}" "${RTTI_FLAG}" ${flag_var} "${${flag_var}}")
ENDFOREACH()

add_subdirectory(FreeImageEntry)
add_subdirectory(LibJPEG)
add_subdirectory(LibJXR)
add_subdirectory(LibOpenJPEG)
add_subdirectory(LibPNG)
add_subdirectory(LibRaw)
add_subdirectory(LibTIFF4)
add_subdirectory(LibWebP)
add_subdirectory(OpenEXR)

foreach(target
	"FreeImage" "LibJPEG" "LibJXR" "LibOpenJPEG" "LibPNG" "LibRaw" "LibTIFF4" "LibWebP" "OpenEXR")
	if(ZENGINE_PLATFORM_WINDOWS)
		target_compile_definitions(${target}
			PRIVATE
				-DWIN32
		)
	else()
		target_compile_definitions(${target}
			PRIVATE
				-D__ANSI__
		)
	endif()

	set_target_properties(${target} PROPERTIES FOLDER "3rd/FreeImage")
endforeach()
