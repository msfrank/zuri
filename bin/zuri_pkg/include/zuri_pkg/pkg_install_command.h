#ifndef ZURI_PKG_PKG_INSTALL_COMMAND_H
#define ZURI_PKG_PKG_INSTALL_COMMAND_H

#include <tempo_command/command_tokenizer.h>
#include <tempo_utils/status.h>
#include <zuri_distributor/runtime.h>
#include <zuri_tooling/environment_config.h>

namespace zuri_pkg {
    tempo_utils::Status pkg_install_command(
        std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
        std::shared_ptr<zuri_distributor::Runtime> runtime,
        tempo_command::TokenVector &tokens);
}

#endif // ZURI_PKG_PKG_INSTALL_COMMAND_H