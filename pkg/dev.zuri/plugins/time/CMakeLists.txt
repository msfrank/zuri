
set(PLUGIN_NAME       "time")
set(PLUGIN_BASENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}")
set(PLUGIN_FILENAME   "${PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-time plugin
add_library(std-time-plugin SHARED
    datetime_ref.cpp
    datetime_ref.h
    instant_ref.cpp
    instant_ref.h
    native_time.cpp
    native_time.h
    time_traps.cpp
    time_traps.h
    timezone_ref.cpp
    timezone_ref.h
    )

set_target_properties(std-time-plugin PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the plugin RPATH
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-time-plugin PROPERTIES INSTALL_RPATH "@loader_path/../runtime-lib;@loader_path/../lib")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(std-time-plugin PROPERTIES INSTALL_RPATH "$ORIGIN/../runtime-lib;$ORIGIN/../lib")
endif()

target_link_libraries(std-time-plugin PUBLIC
    lyric::lyric_runtime
    Boost::headers
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
