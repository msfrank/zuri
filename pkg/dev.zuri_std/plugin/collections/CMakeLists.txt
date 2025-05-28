
set(PLUGIN_NAME       "collections")
set(PLUGIN_BASENAME   "${PLUGIN_NAME}.${ZURI_PLATFORM_ID}")
set(PLUGIN_FILENAME   "${PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(PLUGIN_FILEPATH   "${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_FILENAME}")

# build the std-collections plugin
add_library(std-collections-plugin SHARED
    hashmap_key.h
    hashmap_ref.cpp
    hashmap_ref.h
    hashmap_traps.cpp
    hashmap_traps.h
    native_collections.cpp
    native_collections.h
    treemap_ref.cpp
    treemap_ref.h
    treemap_traps.cpp
    treemap_traps.h
    treeset_ref.cpp
    treeset_ref.h
    treeset_traps.cpp
    treeset_traps.h
    vector_ref.cpp
    vector_ref.h
    vector_traps.cpp
    vector_traps.h
    )

#
set_target_properties(std-collections-plugin PROPERTIES
    INSTALL_RPATH_USE_LINK_PATH TRUE
    LIBRARY_OUTPUT_NAME ${PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-collections-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

target_link_libraries(std-collections-plugin PUBLIC
    lyric::lyric_runtime
    absl::btree
    absl::flat_hash_map
    absl::inlined_vector
    )

set_directory_properties(PROPERTIES
    PLUGIN_NAME ${PLUGIN_NAME}
    PLUGIN_FILENAME ${PLUGIN_FILENAME}
    PLUGIN_FILEPATH ${PLUGIN_FILEPATH}
    )
