
set(LIB_NAME "text")

set(ZURI_STD_TEXT_MODULE_NAME       "${LIB_NAME}.lyo")
set(ZURI_STD_TEXT_MODULE_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_TEXT_MODULE_NAME}")
set(ZURI_STD_TEXT_PLUGIN_BASENAME   "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_TEXT_PLUGIN_NAME       "${ZURI_STD_TEXT_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(ZURI_STD_TEXT_PLUGIN_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_TEXT_PLUGIN_NAME}")

set(ZURI_STD_TEXT_LOCATION          "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# build the text module

add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/compile_module/src/text.lyo
    COMMAND
      zuri-build -vv
        -W ${CMAKE_CURRENT_SOURCE_DIR}
        -B ${CMAKE_CURRENT_BINARY_DIR}
        -I ${CMAKE_CURRENT_BINARY_DIR}
        compile_module:src/text.ly
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      zuri-build
)

add_custom_target(std-text-module
    COMMAND
      ${CMAKE_COMMAND}
        -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/compile_module/src/text.lyo" "${ZURI_STD_TEXT_MODULE_PATH}"
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/compile_module/src/text.lyo
)

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_text/config.h
    @ONLY
    )

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

set_target_properties(std-text-plugin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(std-text-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-text-plugin PRIVATE
    lyric::lyric_runtime
    )

add_custom_target(ZuriStdText DEPENDS std-text-module std-text-plugin)

# install targets
install(TARGETS std-text-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_TEXT_MODULE_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
