#ifndef ZURI_ENV_ENV_INSTALL_COMMAND_H
#define ZURI_ENV_ENV_INSTALL_COMMAND_H

#include <tempo_command/command_tokenizer.h>
#include <tempo_utils/status.h>
#include <zuri_tooling/zuri_config.h>

namespace zuri_env {

    tempo_utils::Status env_create_command(
        std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
        tempo_command::TokenVector &tokens);

    tempo_utils::Status symlink_directory(
        const std::filesystem::path &srcDirectory,
        const std::filesystem::path &dstDirectory);
}

#endif // ZURI_ENV_ENV_INSTALL_COMMAND_H