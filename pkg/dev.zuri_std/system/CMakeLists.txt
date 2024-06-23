
set(LIB_NAME        "system")

set(ZURI_STD_SYSTEM_ASSEMBLY_NAME     "${LIB_NAME}.lyo")
set(ZURI_STD_SYSTEM_ASSEMBLY_PATH     "${PACKAGE_BUILD_DIR}/${ZURI_STD_SYSTEM_ASSEMBLY_NAME}")
set(ZURI_STD_SYSTEM_PLUGIN_BASENAME   "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_SYSTEM_PLUGIN_NAME       "${ZURI_STD_SYSTEM_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(ZURI_STD_SYSTEM_PLUGIN_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_SYSTEM_PLUGIN_NAME}")

set(ZURI_STD_SYSTEM_LOCATION          "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_system/config.h
    @ONLY
    )

add_executable(std-system-builder
    assembly/compile_attr.cpp
    assembly/compile_attr.h
    assembly/compile_element.cpp
    assembly/compile_element.h
    assembly/compile_future.cpp
    assembly/compile_future.h
    assembly/compile_operation.cpp
    assembly/compile_operation.h
    assembly/compile_port.cpp
    assembly/compile_port.h
    assembly/compile_queue.cpp
    assembly/compile_queue.h
    assembly/compile_system.cpp
    assembly/compile_system.h
    assembly/lib_builder.cpp
    include/zuri_std_system/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_system/config.h
    )

target_include_directories(std-system-builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_compile_definitions(std-system-builder PRIVATE
    "ZURI_BUILD_PACKAGES_DIR=\"${ZURI_BUILD_PACKAGES_DIR}\""
    "DEFAULT_DESTINATION_PATH=\"${ZURI_STD_SYSTEM_ASSEMBLY_PATH}\""
    )

target_link_libraries(std-system-builder
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    )

# build assembly file
add_custom_command (
    OUTPUT ${ZURI_STD_SYSTEM_ASSEMBLY_PATH}
    COMMAND std-system-builder ${ZURI_STD_SYSTEM_ASSEMBLY_PATH}
    COMMENT "generating ${ZURI_STD_SYSTEM_ASSEMBLY_NAME}"
    DEPENDS std-system-builder
)
add_custom_target(std-system-assembly DEPENDS ${ZURI_STD_SYSTEM_ASSEMBLY_PATH})

# build system plugin as a shared library
add_library(std-system-plugin SHARED
    plugin/attr_ref.cpp
    plugin/attr_ref.h
    plugin/element_ref.cpp
    plugin/element_ref.h
    plugin/future_ref.cpp
    plugin/future_ref.h
    plugin/native_system.cpp
    plugin/native_system.h
    plugin/operation_ref.cpp
    plugin/operation_ref.h
    plugin/port_ref.cpp
    plugin/port_ref.h
    plugin/queue_ref.cpp
    plugin/queue_ref.h
    plugin/system_traps.cpp
    plugin/system_traps.h
    include/zuri_std_system/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_system/config.h
    )

#
set_target_properties(std-system-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${ZURI_STD_SYSTEM_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${PACKAGE_BUILD_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-system-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(std-system-plugin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(std-system-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-system-plugin
    lyric::lyric_runtime
    )

add_custom_target(ZuriStdSystem DEPENDS std-system-assembly std-system-plugin)

# install targets
install(TARGETS std-system-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_SYSTEM_ASSEMBLY_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
