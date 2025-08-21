#ifndef ZURI_RUN_RUN_PACKAGE_COMMAND_H
#define ZURI_RUN_RUN_PACKAGE_COMMAND_H

#include <tempo_utils/status.h>
#include <zuri_tooling/zuri_config.h>

namespace zuri_run {
    tempo_utils::Status run_package_command(
        std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
        const std::string &sessionId,
        const std::filesystem::path &mainPackagePath,
        const std::vector<std::string> &mainArgs);
}

#endif // ZURI_RUN_RUN_PACKAGE_COMMAND_H