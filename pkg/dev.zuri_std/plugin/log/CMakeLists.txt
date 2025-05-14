
set(PLUGIN_NAME       "log")
set(PLUGIN_FILENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-log plugin
add_library(std-log-plugin MODULE
    log_traps.cpp
    log_traps.h
    native_log.cpp
    native_log.h
    )

#
set_target_properties(std-log-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_FILENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-log-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_link_libraries(std-log-plugin PUBLIC
    lyric::lyric_runtime
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )