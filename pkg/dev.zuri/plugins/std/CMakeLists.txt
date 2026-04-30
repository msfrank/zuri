
set(PLUGIN_NAME       "std")
set(PLUGIN_BASENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}")
set(PLUGIN_FILENAME   "${PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std plugin
add_library(std-plugin SHARED
    datetime_ref.cpp
    datetime_ref.h
    future_ref.cpp
    future_ref.h
    hashmap_key.h
    hashmap_ref.cpp
    hashmap_ref.h
    hashmap_traps.cpp
    hashmap_traps.h
    instant_ref.cpp
    instant_ref.h
    native_std.cpp
    native_std.h
    port_ref.cpp
    port_ref.h
    process_traps.cpp
    process_traps.h
    resource_traps.cpp
    resource_traps.h
    system_traps.cpp
    system_traps.h
    time_traps.cpp
    time_traps.h
    timezone_ref.cpp
    timezone_ref.h
    treemap_ref.cpp
    treemap_ref.h
    treemap_traps.cpp
    treemap_traps.h
    treeset_ref.cpp
    treeset_ref.h
    treeset_traps.cpp
    treeset_traps.h
    url_ref.cpp
    url_ref.h
    vector_ref.cpp
    vector_ref.h
    vector_traps.cpp
    vector_traps.h
    work_queue_ref.cpp
    work_queue_ref.h
    )

set_target_properties(std-plugin PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the plugin RPATH
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-plugin PROPERTIES INSTALL_RPATH "@loader_path/../runtime-lib;@loader_path/../lib")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set_target_properties(std-plugin PROPERTIES INSTALL_RPATH "$ORIGIN/../runtime-lib;$ORIGIN/../lib")
endif()

target_link_libraries(std-plugin PUBLIC
    lyric::lyric_runtime
    Boost::headers
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
