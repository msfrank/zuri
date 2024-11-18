
set(LIB_NAME "time")

set(ZURI_STD_TIME_MODULE_NAME       "${LIB_NAME}.lyo")
set(ZURI_STD_TIME_MODULE_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_TIME_MODULE_NAME}")
set(ZURI_STD_TIME_PLUGIN_BASENAME   "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_TIME_PLUGIN_NAME       "${ZURI_STD_TIME_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(ZURI_STD_TIME_PLUGIN_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_TIME_PLUGIN_NAME}")

set(ZURI_STD_TIME_LOCATION          "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_time/config.h
    @ONLY
    )

add_executable(std-time-builder
    assembly/compile_datetime.cpp
    assembly/compile_datetime.h
    assembly/compile_instant.cpp
    assembly/compile_instant.h
    assembly/compile_timezone.cpp
    assembly/compile_timezone.h
    assembly/compile_time.cpp
    assembly/compile_time.h
    assembly/lib_builder.cpp
    include/zuri_std_time/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_time/config.h
    )

target_include_directories(std-time-builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_compile_definitions(std-time-builder PRIVATE
    "ZURI_BUILD_PACKAGES_DIR=\"${ZURI_BUILD_PACKAGES_DIR}\""
    "DEFAULT_DESTINATION_PATH=\"${ZURI_STD_TIME_MODULE_PATH}\""
    )

target_link_libraries(std-time-builder
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    )

# build assembly file
add_custom_command (
    OUTPUT ${ZURI_STD_TIME_MODULE_PATH}
    COMMAND std-time-builder ${ZURI_STD_TIME_MODULE_PATH}
    COMMENT "generating ${ZURI_STD_TIME_MODULE_NAME}"
    DEPENDS std-time-builder
    )
add_custom_target(std-time-assembly DEPENDS ${ZURI_STD_TIME_MODULE_PATH})

# build string plugin as a shared library
add_library(std-time-plugin SHARED
    plugin/datetime_ref.cpp
    plugin/datetime_ref.h
    plugin/native_time.cpp
    plugin/native_time.h
    plugin/instant_ref.cpp
    plugin/instant_ref.h
    plugin/timezone_ref.cpp
    plugin/timezone_ref.h
    plugin/time_traps.cpp
    plugin/time_traps.h
    include/zuri_std_time/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_time/config.h
    )

#
set_target_properties(std-time-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${ZURI_STD_TIME_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${PACKAGE_BUILD_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-time-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(std-time-plugin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(std-time-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-time-plugin PRIVATE
    lyric::lyric_runtime
    )

add_custom_target(ZuriStdTime DEPENDS std-time-assembly std-time-plugin)

# install targets
install(TARGETS std-time-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_TIME_MODULE_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
