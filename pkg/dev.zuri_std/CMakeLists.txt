
set(PACKAGE_BUILD_DIR               "${ZURI_BUILD_PACKAGES_DIR}/${ZURI_STD_PACKAGE_ID}")
set(PACKAGE_INSTALL_DIR             "${PACKAGES_DIR_PREFIX}/${ZURI_STD_PACKAGE_ID}")

# declare plugin targets
add_subdirectory(plugin)

# get file paths to each plugin
get_directory_property(ZURI_STD_COLLECTIONS_PLUGIN_FILEPATH DIRECTORY plugin/collections PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_LOG_PLUGIN_FILEPATH DIRECTORY plugin/log PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_SYSTEM_PLUGIN_FILEPATH DIRECTORY plugin/system PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_TEXT_PLUGIN_FILEPATH DIRECTORY plugin/text PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_TIME_PLUGIN_FILEPATH DIRECTORY plugin/time PLUGIN_FILEPATH)

# create the workspace config file used to build package
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/workspace.config.in ${CMAKE_CURRENT_BINARY_DIR}/workspace.config @ONLY)

set(ZURI_STD_PACKAGE_PATH ${CMAKE_CURRENT_BINARY_DIR}/dev.zuri_std-${PROJECT_VERSION}.zpk)

# build the std package
add_custom_command (
    OUTPUT
        ${ZURI_STD_PACKAGE_PATH}
    COMMAND
        zuri-build -v
        -W ${CMAKE_CURRENT_SOURCE_DIR}
        --workspace-config-file ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
        -B ${CMAKE_CURRENT_BINARY_DIR}
        -I ${CMAKE_CURRENT_BINARY_DIR}
        --distribution-root ${ZURI_BUILD_OUTPUT_ROOT}
        "dev.zuri_std"
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        zuri-build
        ${CMAKE_CURRENT_BINARY_DIR}/workspace.config
        std-collections-plugin
        std-log-plugin
        std-system-plugin
        std-text-plugin
        std-time-plugin
    )

add_custom_target(zuri-std-package DEPENDS ${ZURI_STD_PACKAGE_PATH})

add_subdirectory(test)