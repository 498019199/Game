SET(SEVENZIP_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/7z")

# kornelski/7z history was rewritten (repo archived); pin to 25.00 which still matches our file list.
UpdateExternalLib("7z" "https://github.com/kornelski/7z" "7dc9dc83ec39e6dc3edf3eef942015736bc0a300" "" "" need_patch)
if(need_patch)
	foreach(patch "0001-Support-UWP" "0002-Fix-for-clang-cl" "0003-Fix-for-Android-build")
		ApplyPatch("7z" "${CMAKE_CURRENT_SOURCE_DIR}/Patches/${patch}.patch")
	endforeach()
endif()

add_subdirectory(7zxa)
add_subdirectory(LZMA)
