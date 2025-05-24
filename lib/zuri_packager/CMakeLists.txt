
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build zuri_packager as a shared library
add_library(zuri_packager SHARED)
add_library(zuri::zuri_packager ALIAS zuri_packager)

set(ZURI_PACKAGER_INCLUDES
    include/zuri_packager/entry_path.h
    include/zuri_packager/entry_walker.h
    include/zuri_packager/zuri_manifest.h
    include/zuri_packager/manifest_attr.h
    include/zuri_packager/manifest_attr_parser.h
    include/zuri_packager/manifest_attr_writer.h
    include/zuri_packager/manifest_entry.h
    include/zuri_packager/manifest_namespace.h
    include/zuri_packager/manifest_state.h
    include/zuri_packager/manifest_walker.h
    include/zuri_packager/package_attrs.h
    include/zuri_packager/package_reader.h
    include/zuri_packager/package_result.h
    include/zuri_packager/package_specifier.h
    include/zuri_packager/package_types.h
    include/zuri_packager/package_writer.h
    include/zuri_packager/packaging_conversions.h
    )
set_target_properties(zuri_packager PROPERTIES PUBLIC_HEADER "${ZURI_PACKAGER_INCLUDES}")

# generate flatbuffer files for manifest IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated/manifest.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
)

# generate header containing the manifest schema
add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated/manifest_schema.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated
    COMMAND
      tempo::tempo-bytes2code
      -g ZURI_PACKAGER_GENERATED_MANIFEST_SCHEMA_H -n zuri_packager::schema::manifest
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
      ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated/manifest_schema.h
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
)

target_sources(zuri_packager PRIVATE
    src/entry_path.cpp
    src/entry_walker.cpp
    src/zuri_manifest.cpp
    src/manifest_attr.cpp
    src/manifest_attr_parser.cpp
    src/manifest_attr_writer.cpp
    src/manifest_entry.cpp
    src/manifest_namespace.cpp
    src/manifest_state.cpp
    src/manifest_walker.cpp
    src/package_attrs.cpp
    src/package_reader.cpp
    src/package_result.cpp
    src/package_specifier.cpp
    src/package_types.cpp
    src/package_writer.cpp
    src/packaging_conversions.cpp

    include/zuri_packager/internal/manifest_reader.h
    src/internal/manifest_reader.cpp

    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated/manifest.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_packager/generated/manifest_schema.h
    )

# set the library version
set_target_properties(zuri_packager PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(zuri_packager PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(zuri_packager PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${ZURI_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(zuri_packager PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# make private headers visible internally
target_include_directories(zuri_packager PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(zuri_packager
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    PRIVATE
    flatbuffers::flatbuffers
    )

# install targets
install(TARGETS zuri_packager EXPORT zuri-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zuri_packager
    )

# install data
install(FILES share/manifest.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/zuri_packager)

# add testing subdirectory
add_subdirectory(test)
