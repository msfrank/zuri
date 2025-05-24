
# do not run moc automatically
set(CMAKE_AUTOMOC OFF)

# build zuri_distributor as a shared library
add_library(zuri_distributor SHARED)
add_library(zuri::zuri_distributor ALIAS zuri_distributor)

set(ZURI_DISTRIBUTOR_INCLUDES
    include/zuri_distributor/package_config.h
    )
set_target_properties(zuri_distributor PROPERTIES PUBLIC_HEADER "${ZURI_DISTRIBUTOR_INCLUDES}")

# generate flatbuffer files for manifest IDL
add_custom_command (
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_distributor/generated/manifest.h
    COMMAND
      cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_distributor/generated
    COMMAND
      ${FLATC} --cpp --scoped-enums --gen-mutable --gen-object-api --filename-suffix ''
      -o ${CMAKE_CURRENT_BINARY_DIR}/include/zuri_distributor/generated
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
    DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/share/manifest.fbs
)

target_sources(zuri_distributor PRIVATE
    src/package_config.cpp
    )

# set the library version
set_target_properties(zuri_distributor PROPERTIES VERSION "${FULL_VERSION}" SOVERSION "${MAJOR_VERSION}")

# set the RPATH if on OS X
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(zuri_distributor PROPERTIES MACOSX_RPATH TRUE)
endif()

set_target_properties(zuri_distributor PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${ZURI_BUILD_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH TRUE
    INSTALL_RPATH ${LIB_RPATH}
    )

# set the public header include path differently on the target depending on the interface
target_include_directories(zuri_distributor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

# make private headers visible internally
target_include_directories(zuri_distributor PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src)

target_link_libraries(zuri_distributor
    PUBLIC
    lyric::lyric_runtime
    lyric::lyric_schema
    tempo::tempo_utils
    zuri::zuri_packager
    )

# install targets
install(TARGETS zuri_distributor EXPORT zuri-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/zuri_distributor
    )

# install data
install(FILES share/manifest.fbs DESTINATION ${CMAKE_INSTALL_DATADIR}/zuri_distributor)

# add testing subdirectory
#add_subdirectory(test)
