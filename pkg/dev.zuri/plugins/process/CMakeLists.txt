
set(PLUGIN_NAME       "process")
set(PLUGIN_BASENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}")
set(PLUGIN_FILENAME   "${PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-process plugin
add_library(std-process-plugin SHARED
    native_process.cpp
    native_process.h
    process_traps.cpp
    process_traps.h
    )

set_target_properties(std-process-plugin PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the plugin RPATH
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-process-plugin PROPERTIES INSTALL_RPATH "@loader_path/../runtime-lib;@loader_path/../lib")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(std-process-plugin PROPERTIES INSTALL_RPATH "$ORIGIN/../runtime-lib;$ORIGIN/../lib")
endif()

target_link_libraries(std-process-plugin PUBLIC
    lyric::lyric_runtime
    Boost::headers
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
