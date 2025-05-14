
set(PLUGIN_NAME       "time")
set(PLUGIN_FILENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-time plugin
add_library(std-time-plugin MODULE
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

#
set_target_properties(std-time-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_FILENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
)

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-time-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_link_libraries(std-time-plugin PUBLIC
    lyric::lyric_runtime
)

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
)
