
set(LIB_NAME        "collections")

set(ZURI_STD_COLLECTIONS_MODULE_NAME       "${LIB_NAME}.lyo")
set(ZURI_STD_COLLECTIONS_MODULE_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_COLLECTIONS_MODULE_NAME}")
set(ZURI_STD_COLLECTIONS_PLUGIN_BASENAME   "${LIB_NAME}.${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
set(ZURI_STD_COLLECTIONS_PLUGIN_NAME       "${ZURI_STD_COLLECTIONS_PLUGIN_BASENAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(ZURI_STD_COLLECTIONS_PLUGIN_PATH       "${PACKAGE_BUILD_DIR}/${ZURI_STD_COLLECTIONS_PLUGIN_NAME}")

set(ZURI_STD_COLLECTIONS_LOCATION          "${ZURI_STD_PACKAGE_URL}/${LIB_NAME}")

# create the platform configuration header file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_collections/config.h
    @ONLY
    )

add_executable(std-collections-builder
    assembly/compile_hashmap.cpp
    assembly/compile_hashmap.h
    assembly/compile_option.cpp
    assembly/compile_option.h
    assembly/compile_treemap.cpp
    assembly/compile_treemap.h
    assembly/compile_treeset.cpp
    assembly/compile_treeset.h
    assembly/compile_treeset_iterator.cpp
    assembly/compile_treeset_iterator.h
    assembly/compile_vector.cpp
    assembly/compile_vector.h
    assembly/compile_vector_iterator.cpp
    assembly/compile_vector_iterator.h
    assembly/lib_builder.cpp
    include/zuri_std_collections/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_collections/config.h
    )

target_include_directories(std-collections-builder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_definitions(std-collections-builder PRIVATE
    "ZURI_BUILD_PACKAGES_DIR=\"${ZURI_BUILD_PACKAGES_DIR}\""
    "DEFAULT_DESTINATION_PATH=\"${ZURI_STD_COLLECTIONS_MODULE_PATH}\""
    )

target_link_libraries(std-collections-builder
    lyric::lyric_build
    tempo::tempo_command
    tempo::tempo_utils
    )

# build assembly file
add_custom_command (
    OUTPUT ${ZURI_STD_COLLECTIONS_MODULE_PATH}
    COMMAND std-collections-builder ${ZURI_STD_COLLECTIONS_MODULE_PATH}
    COMMENT "generating ${ZURI_STD_COLLECTIONS_MODULE_NAME}"
    DEPENDS std-collections-builder
)
add_custom_target(std-collections-assembly DEPENDS ${ZURI_STD_COLLECTIONS_MODULE_PATH})

# build collections plugin as a shared library
add_library(std-collections-plugin SHARED
    plugin/hashmap_key.h
    plugin/hashmap_ref.cpp
    plugin/hashmap_ref.h
    plugin/hashmap_traps.cpp
    plugin/hashmap_traps.h
    plugin/native_collections.cpp
    plugin/native_collections.h
    plugin/treemap_ref.cpp
    plugin/treemap_ref.h
    plugin/treemap_traps.cpp
    plugin/treemap_traps.h
    plugin/treeset_ref.cpp
    plugin/treeset_ref.h
    plugin/treeset_traps.cpp
    plugin/treeset_traps.h
    plugin/vector_ref.cpp
    plugin/vector_ref.h
    plugin/vector_traps.cpp
    plugin/vector_traps.h
    include/zuri_std_collections/lib_types.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_std_collections/config.h
    )

#
set_target_properties(std-collections-plugin PROPERTIES
    LIBRARY_OUTPUT_NAME ${ZURI_STD_COLLECTIONS_PLUGIN_BASENAME}
    LIBRARY_OUTPUT_DIRECTORY ${PACKAGE_BUILD_DIR}
    PREFIX ""
    )

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(std-collections-plugin PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(std-collections-plugin PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

target_include_directories(std-collections-plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

target_link_libraries(std-collections-plugin PRIVATE
    lyric::lyric_runtime
    absl::btree
    absl::flat_hash_map
    absl::inlined_vector
    )

add_custom_target(ZuriStdCollections DEPENDS std-collections-assembly std-collections-plugin)

# install targets
install(TARGETS std-collections-plugin LIBRARY DESTINATION ${PACKAGE_INSTALL_DIR})
install(FILES ${ZURI_STD_COLLECTIONS_MODULE_PATH} DESTINATION ${PACKAGE_INSTALL_DIR})

# add testing subdirectory
add_subdirectory(test)
