
set(PLUGIN_NAME       "system")
set(PLUGIN_FILENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-system plugin
add_library(std-system-plugin MODULE
    attr_ref.cpp
    attr_ref.h
    element_ref.cpp
    element_ref.h
    future_ref.cpp
    future_ref.h
    native_system.cpp
    native_system.h
    operation_ref.cpp
    operation_ref.h
    port_ref.cpp
    port_ref.h
    queue_ref.cpp
    queue_ref.h
    system_traps.cpp
    system_traps.h
    )

#
set_target_properties(std-system-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_FILENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-system-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_link_libraries(std-system-plugin PUBLIC
    lyric::lyric_runtime
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
