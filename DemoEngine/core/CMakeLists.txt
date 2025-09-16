
set(TableGenOutput "${ZENGINE_ROOT_DIR}/core/core/src/TableGen/Tables.h" "${ZENGINE_ROOT_DIR}/core/core/src/TableGen/Tables.cpp")
add_custom_command(OUTPUT ${TableGenOutput}
	COMMAND "${Python3_EXECUTABLE}" "TableGen.py"
	WORKING_DIRECTORY "${ZENGINE_ROOT_DIR}/core/core/src/TableGen/"
	COMMENT "Generating tables...")
add_custom_target(TableGen DEPENDS ${TableGenOutput})
set_target_properties(TableGen PROPERTIES 
    FOLDER "ZENGINE"  # 将目标放在 ZENGINE 文件夹下
)

add_subdirectory(core)
add_subdirectory(plungins)