SET(LIB_NAME 7zxa)

SET(7ZIP_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/ICoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IDecl.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IPassword.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IProgress.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/IStream.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/PropID.h
)
SET(7ZIP_COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CreateCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CWrappers.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/FilterCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/InBuffer.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LimitedStreams.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodId.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodProps.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/OutBuffer.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/ProgressUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/PropId.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamObjects.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CreateCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/CWrappers.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/FilterCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/InBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/LimitedStreams.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodId.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/MethodProps.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/OutBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/ProgressUtils.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/RegisterArc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/RegisterCodec.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamObjects.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Common/StreamUtils.h
)
SET(ARCHIVE_7Z_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zCompressionMode.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zDecode.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zExtract.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHandler.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHandlerOut.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHeader.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zIn.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zProperties.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zCompressionMode.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zDecode.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHandler.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zHeader.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zIn.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zItem.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/7zProperties.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/StdAfx.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/StdAfx.cpp
)
SET(ARCHIVE_COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/HandlerOut.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ParseProperties.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/CoderMixer2.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/HandlerOut.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ItemNameUtils.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/OutStreamWithCRC.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Common/ParseProperties.h
)
SET(ARCHIVE_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/IArchive.h
)
SET(C_FILES
	${SEVENZIP_PROJECT_DIR}/C/7zCrc.c
	${SEVENZIP_PROJECT_DIR}/C/7zCrcOpt.c
	${SEVENZIP_PROJECT_DIR}/C/Aes.c
	${SEVENZIP_PROJECT_DIR}/C/AesOpt.c
	${SEVENZIP_PROJECT_DIR}/C/Alloc.c
	${SEVENZIP_PROJECT_DIR}/C/Bcj2.c
	${SEVENZIP_PROJECT_DIR}/C/Bra.c
	${SEVENZIP_PROJECT_DIR}/C/Bra86.c
	${SEVENZIP_PROJECT_DIR}/C/BraIA64.c
	${SEVENZIP_PROJECT_DIR}/C/CpuArch.c
	${SEVENZIP_PROJECT_DIR}/C/Delta.c
	${SEVENZIP_PROJECT_DIR}/C/Lzma2Dec.c
	${SEVENZIP_PROJECT_DIR}/C/Lzma2DecMt.c
	${SEVENZIP_PROJECT_DIR}/C/LzmaDec.c
	${SEVENZIP_PROJECT_DIR}/C/MtDec.c
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7.c
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7Dec.c
	${SEVENZIP_PROJECT_DIR}/C/Sha256.c
	${SEVENZIP_PROJECT_DIR}/C/Sha256Opt.c
	${SEVENZIP_PROJECT_DIR}/C/SwapBytes.c
	${SEVENZIP_PROJECT_DIR}/C/7zCrc.h
	${SEVENZIP_PROJECT_DIR}/C/Aes.h
	${SEVENZIP_PROJECT_DIR}/C/Alloc.h
	${SEVENZIP_PROJECT_DIR}/C/Bcj2.h
	${SEVENZIP_PROJECT_DIR}/C/Bra.h
	${SEVENZIP_PROJECT_DIR}/C/CpuArch.h
	${SEVENZIP_PROJECT_DIR}/C/Delta.h
	${SEVENZIP_PROJECT_DIR}/C/Lzma2Dec.h
	${SEVENZIP_PROJECT_DIR}/C/Lzma2DecMt.h
	${SEVENZIP_PROJECT_DIR}/C/LzmaDec.h
	${SEVENZIP_PROJECT_DIR}/C/MtDec.h
	${SEVENZIP_PROJECT_DIR}/C/Ppmd.h
	${SEVENZIP_PROJECT_DIR}/C/Ppmd7.h
	${SEVENZIP_PROJECT_DIR}/C/RotateDefs.h
	${SEVENZIP_PROJECT_DIR}/C/Sha256.h
	${SEVENZIP_PROJECT_DIR}/C/SwapBytes.h
	${SEVENZIP_PROJECT_DIR}/C/Threads.h
)
SET(COMMON_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/Common/CRC.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/CrcReg.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/IntToString.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyString.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyVector.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyWindows.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/NewHandler.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Sha256Reg.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringConvert.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringToInt.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/UTFConvert.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Wildcard.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Common/DynamicBuffer.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/IntToString.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyCom.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyException.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyInitGuid.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyString.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyUnknown.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyVector.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/MyWindows.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/NewHandler.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringConvert.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/StringToInt.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/UTFConvert.h
	${SEVENZIP_PROJECT_DIR}/CPP/Common/Wildcard.h
)
SET(7Z_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Coder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchMisc.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/ByteSwap.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CodecExports.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyCoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Decoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Bcj2Coder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BcjCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BranchMisc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/CopyCoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Lzma2Decoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzmaDecoder.h
)
SET(BIT_CODER_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitlEncoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitmDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BitmEncoder.h
)
SET(BZIP2_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Crc.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Decoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Register.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Const.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Crc.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/BZip2Decoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/Mtf8.h
)
SET(PPMD_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/PpmdDecoder.h
)
SET(ZIP_COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateDecoder.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateConst.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeflateDecoder.h
)
SET(COMPRESS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/DeltaFilter.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzOutWindow.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/HuffmanDecoder.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Compress/LzOutWindow.h
)
SET(CRYPTO_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAes.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAesRegister.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/MyAes.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/MyAesReg.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/7zAes.h
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Crypto/MyAes.h
)
SET(SPEC_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/Archive2.def
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/ArchiveExports.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/DllExports2.cpp
)
SET(WINDOWS_FILES
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/PropVariant.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/System.cpp
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/Defs.h
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/PropVariant.h
	${SEVENZIP_PROJECT_DIR}/CPP/Windows/System.h
)

SOURCE_GROUP("7zip" FILES ${7ZIP_FILES})
SOURCE_GROUP("7zip Common" FILES ${7ZIP_COMMON_FILES})
SOURCE_GROUP("Archive\\7z" FILES ${ARCHIVE_7Z_FILES})
SOURCE_GROUP("Archive\\Archive common" FILES ${ARCHIVE_COMMON_FILES})
SOURCE_GROUP("Archive" FILES ${ARCHIVE_FILES})
SOURCE_GROUP("C" FILES ${C_FILES})
SOURCE_GROUP("Common" FILES ${COMMON_FILES})
SOURCE_GROUP("Compress\\7z Compress" FILES ${7Z_COMPRESS_FILES})
SOURCE_GROUP("Compress\\Bit Coder" FILES ${BIT_CODER_FILES})
SOURCE_GROUP("Compress\\BZip2 Compress" FILES ${BZIP2_COMPRESS_FILES})
SOURCE_GROUP("Compress\\PPMD" FILES ${PPMD_FILES})
SOURCE_GROUP("Compress\\Zip Compress" FILES ${ZIP_COMPRESS_FILES})
SOURCE_GROUP("Compress" FILES ${COMPRESS_FILES})
SOURCE_GROUP("Crypto" FILES ${CRYPTO_FILES})
SOURCE_GROUP("Spec" FILES ${SPEC_FILES})
SOURCE_GROUP("Windows" FILES ${WINDOWS_FILES})

ADD_LIBRARY(${LIB_NAME} ${ZENGINE_PREFERRED_LIB_TYPE}
	${7ZIP_FILES} ${7ZIP_COMMON_FILES} ${ARCHIVE_7Z_FILES} ${ARCHIVE_COMMON_FILES}
	${ARCHIVE_FILES} ${C_FILES} ${COMMON_FILES} ${7Z_COMPRESS_FILES} ${BIT_CODER_FILES}
	${BZIP2_COMPRESS_FILES} ${PPMD_FILES} ${ZIP_COMPRESS_FILES} ${COMPRESS_FILES}
	${CRYPTO_FILES} ${SPEC_ICONS_FILES} ${SPEC_FILES} ${WINDOWS_FILES}
)

if(ZENGINE_COMPILER_CLANGCL)
	target_compile_options(${LIB_NAME}
		PRIVATE
			-Wno-dangling-else
			-Wno-delete-non-abstract-non-virtual-dtor
			-Wno-logical-op-parentheses
			-Wno-microsoft-exception-spec
			-Wno-reorder-ctor
			-Wno-sign-compare
			-Wno-unused-function
			-Wno-unused-private-field
	)
endif()

target_compile_definitions(${LIB_NAME}
	PRIVATE
		-DMY7Z_EXPORTS
		-DZ7_NO_REGISTRY
		-DZ7_EXTRACT_ONLY
		-DZ7_ST
)
IF(ZENGINE_PLATFORM_WINDOWS_DESKTOP)
	target_compile_definitions(${LIB_NAME}
		PRIVATE
			-DZ7_LARGE_PAGES
	)
ENDIF()

target_include_directories(${LIB_NAME}
	PUBLIC
		${SEVENZIP_PROJECT_DIR}
	PRIVATE
		${SEVENZIP_PROJECT_DIR}/CPP
		${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES
	OUTPUT_NAME ${LIB_NAME}${ZENGINE_OUTPUT_SUFFIX}
	FOLDER "3rd/7z"
)

IF(ZENGINE_COMPILER_MSVC)
	target_precompile_headers(${LIB_NAME}
		PRIVATE
			"$<$<COMPILE_LANGUAGE:CXX>:${SEVENZIP_PROJECT_DIR}/CPP/7zip/Archive/7z/StdAfx.h>"
	)
ENDIF()

TARGET_LINK_LIBRARIES(${LIB_NAME})
