
# declare plugin targets
add_subdirectory(plugins)

# get file paths to each plugin
get_directory_property(ZURI_FS_FILE_PLUGIN_FILEPATH DIRECTORY plugins/file PLUGIN_FILEPATH)
get_directory_property(ZURI_FS_PATH_PLUGIN_FILEPATH DIRECTORY plugins/path PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_COLLECTIONS_PLUGIN_FILEPATH DIRECTORY plugins/collections PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_LOG_PLUGIN_FILEPATH DIRECTORY plugins/log PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_PROCESS_PLUGIN_FILEPATH DIRECTORY plugins/process PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_SYSTEM_PLUGIN_FILEPATH DIRECTORY plugins/system PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_TEXT_PLUGIN_FILEPATH DIRECTORY plugins/text PLUGIN_FILEPATH)
get_directory_property(ZURI_STD_TIME_PLUGIN_FILEPATH DIRECTORY plugins/time PLUGIN_FILEPATH)

# the linked project directory
set(ZURI_PROJECT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/project")

# the recreate file
set(ZURI_PROJECT_RECREATE_FILE "${CMAKE_CURRENT_BINARY_DIR}/project.recreate")

# create the workspace config file used to build package
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project.config.in ${CMAKE_CURRENT_BINARY_DIR}/project.config @ONLY)

# build list of extra lib dirs to pass to zuri-project link
list(TRANSFORM RUNTIME_LIB_DIRS PREPEND "--extra-lib-dir=" OUTPUT_VARIABLE EXTRA_LIB_DIR_LIST)

# touch the recreate file whenever the build configuration is updated
file(TOUCH ${ZURI_PROJECT_RECREATE_FILE})

# generate the project directory
add_custom_target(generate-project-directory
    COMMAND
      zuri-project -vv link
        --if-existing Recreate
        --recreate-if-file-present ${ZURI_PROJECT_RECREATE_FILE}
        --link-project-config-file ${CMAKE_CURRENT_BINARY_DIR}/project.config
        ${EXTRA_LIB_DIR_LIST}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZURI_PROJECT_DIRECTORY}
    COMMAND
      ${CMAKE_COMMAND} -E rm -f ${ZURI_PROJECT_RECREATE_FILE}
    COMMAND_EXPAND_LISTS
    DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/project.config
      zuri-project
    )

# build the std package
add_custom_target(zuri-std-package
    COMMAND
      zuri-build -vv
        -P ${ZURI_PROJECT_DIRECTORY}
        -I ${CMAKE_BINARY_DIR}
        "std"
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      zuri-build
      generate-project-directory
      std-collections-plugin
      std-log-plugin
      std-process-plugin
      std-system-plugin
      std-text-plugin
      std-time-plugin
    )

# set custom target properties for consuming std
set_target_properties(zuri-std-package PROPERTIES
    ZURI_PKG_FILEPATH ${ZURI_STD_PACKAGE_PATH}
    ZURI_PKG_SPECIFIER "std-${PROJECT_VERSION}@zuri.dev"
    )

# build the fs package
add_custom_target(zuri-fs-package
    COMMAND
      zuri-build -vv
      -P ${ZURI_PROJECT_DIRECTORY}
      -I ${CMAKE_BINARY_DIR}
      "fs"
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      zuri-build
      generate-project-directory
      zuri-std-package
      fs-file-plugin
      fs-path-plugin
    )

# set custom target properties for consuming fs
set_target_properties(zuri-fs-package PROPERTIES
    ZURI_PKG_FILEPATH ${ZURI_FS_PACKAGE_PATH}
    ZURI_PKG_SPECIFIER "fs-${PROJECT_VERSION}@zuri.dev"
    )

# install zpk files
install(
    FILES
      ${ZURI_STD_PACKAGE_PATH}
      ${ZURI_FS_PACKAGE_PATH}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/zpk
    )

# add testing subdirectory
add_subdirectory(test)
