
set(LIB_NAME "text")

set(ZURI_STD_TEXT_ASSEMBLY_NAME     "${LIB_NAME}.lyo")
set(ZURI_STD_TEXT_ASSEMBLY_PATH     "${PACKAGE_BUILD_DIR}/${ZURI_STD_TEXT_ASSEMBLY_NAME}")
set(ZURI_STD_TEXT_PLUGIN_BASENAME   "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_TEXT_PLUGIN_NAME       "${ZURI_STD_TEXT_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(ZURI_STD_TEXT_PLUGIN_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_TEXT_PLUGIN_NAME}")

set(ZURI_STD_TEXT_LOCATION          "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_text/config.h
    @ONLY
    )

add_executable(std-text-builder
    assembly/compile_text.cpp
    assembly/compile_text.h
    assembly/lib_builder.cpp
    include/zuri_std_text/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_text/config.h
    )

target_include_directories(std-text-builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_compile_definitions(std-text-builder PRIVATE
    "ZURI_BUILD_PACKAGES_DIR=\"${ZURI_BUILD_PACKAGES_DIR}\""
    "DEFAULT_DESTINATION_PATH=\"${ZURI_STD_TEXT_ASSEMBLY_PATH}\""
    )

target_link_libraries(std-text-builder
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    )

# build assembly file
add_custom_command (
    OUTPUT ${ZURI_STD_TEXT_ASSEMBLY_PATH}
    COMMAND std-text-builder ${ZURI_STD_TEXT_ASSEMBLY_PATH}
    COMMENT "generating ${ZURI_STD_TEXT_ASSEMBLY_NAME}"
    DEPENDS std-text-builder
    )
add_custom_target(std-text-assembly DEPENDS ${ZURI_STD_TEXT_ASSEMBLY_PATH})

# build string plugin as a shared library
add_library(std-text-plugin SHARED
    plugin/native_text.cpp
    plugin/native_text.h
    plugin/text_ref.cpp
    plugin/text_ref.h
    include/zuri_std_text/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_text/config.h
    )

#
set_target_properties(std-text-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${ZURI_STD_TEXT_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${PACKAGE_BUILD_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-text-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_include_directories(std-text-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-text-plugin PRIVATE
    lyric::lyric_runtime
    )

add_custom_target(ZuriStdText DEPENDS std-text-assembly std-text-plugin)

# install targets
install(TARGETS std-text-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_TEXT_ASSEMBLY_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
