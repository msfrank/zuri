
set(PACKAGE_ID                  "${ZURI_STD_PACKAGE_GROUP}_${ZURI_STD_PACKAGE_NAME}-${ZURI_STD_PACKAGE_VERSION}")
set(PACKAGE_BUILD_DIR           "${ZURI_BUILD_PACKAGES_DIR}/${PACKAGE_ID}")
set(PACKAGE_INSTALL_DIR         "${PACKAGES_DIR_PREFIX}/${PACKAGE_ID}")

# create the workspace config file used for unit testing
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tester.config.in ${TESTER_CONFIG_PATH} @ONLY)

add_subdirectory(collections)
add_subdirectory(log)
add_subdirectory(system)
add_subdirectory(text)

add_custom_target(ZuriStd DEPENDS
    ZuriStdCollections
    ZuriStdLog
    ZuriStdSystem
    ZuriStdText
    )