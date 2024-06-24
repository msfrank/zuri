
# generate config files

configure_file(zuri.config.in ${ZURI_BUILD_CONFIG_DIR}/zuri.config @ONLY)
configure_file(zuri-build.config.in ${ZURI_BUILD_CONFIG_DIR}/zuri-build.config @ONLY)

# install config files

install(FILES ${ZURI_BUILD_CONFIG_DIR}/zuri.config DESTINATION ${CONFIG_DIR_PREFIX})
install(FILES ${ZURI_BUILD_CONFIG_DIR}/zuri-build.config DESTINATION ${CONFIG_DIR_PREFIX})
