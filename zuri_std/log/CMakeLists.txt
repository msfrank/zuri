
set(LIB_NAME        "log")

set(ZURI_STD_LOG_ASSEMBLY_NAME      "${LIB_NAME}.lyo")
set(ZURI_STD_LOG_ASSEMBLY_PATH      "${PACKAGE_BUILD_DIR}/${ZURI_STD_LOG_ASSEMBLY_NAME}")
set(ZURI_STD_LOG_PLUGIN_BASENAME    "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_LOG_PLUGIN_NAME        "${ZURI_STD_LOG_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

set(ZURI_STD_LOG_LOCATION           "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_log/config.h
    @ONLY
    )

add_executable(std-log-builder
    assembly/compile_log.cpp
    assembly/compile_log.h
    assembly/lib_builder.cpp
    include/zuri_std_log/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_log/config.h
    )

target_include_directories(std-log-builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_compile_definitions(std-log-builder PUBLIC
    "ZURI_BUILD_PACKAGES_DIR=\"${ZURI_BUILD_PACKAGES_DIR}\""
    "DEFAULT_DESTINATION_PATH=\"${ZURI_STD_LOG_ASSEMBLY_PATH}\""
    )

target_link_libraries(std-log-builder
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    )

# build assembly file
add_custom_command (
    OUTPUT ${ZURI_STD_LOG_ASSEMBLY_PATH}
    COMMAND std-log-builder ${ZURI_STD_LOG_ASSEMBLY_PATH}
    COMMENT "generating ${ZURI_STD_LOG_ASSEMBLY_NAME}"
    DEPENDS std-log-builder
)
add_custom_target(std-log-assembly DEPENDS ${ZURI_STD_LOG_ASSEMBLY_PATH})

# build log plugin as a shared library
add_library(std-log-plugin SHARED
    plugin/log_traps.cpp
    plugin/log_traps.h
    plugin/native_log.cpp
    plugin/native_log.h
    include/zuri_std_log/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_log/config.h
    )

#
set_target_properties(std-log-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${ZURI_STD_LOG_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${PACKAGE_BUILD_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-log-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_include_directories(std-log-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-log-plugin PRIVATE
    lyric::lyric_runtime
    )

add_custom_target(ZuriStdLog DEPENDS std-log-assembly std-log-plugin)

# install targets
install(TARGETS std-log-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_LOG_ASSEMBLY_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
