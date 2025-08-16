
# generate config files

configure_file(zuri.config.in ${ZURI_BUILD_CONFIG_DIR}/zuri.config @ONLY)

# install config files

install(FILES ${ZURI_BUILD_CONFIG_DIR}/zuri.config DESTINATION ${CONFIG_DIR_PREFIX})
