
set(PLUGIN_NAME       "path")
set(PLUGIN_BASENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}")
set(PLUGIN_FILENAME   "${PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the fs-path plugin
add_library(fs-path-plugin SHARED
    native_path.cpp
    native_path.h
    path_ref.cpp
    path_ref.h
    )

#
set_target_properties(fs-path-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(fs-path-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_link_libraries(fs-path-plugin PUBLIC
    lyric::lyric_runtime
    Boost::headers
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
