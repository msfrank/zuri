#ifndef ZURI_RUN_RUN_PACKAGE_COMMAND_H
#define ZURI_RUN_RUN_PACKAGE_COMMAND_H

#include <tempo_utils/status.h>
#include <zuri_tooling/environment_config.h>
#include <zuri_tooling/build_tool_config.h>

namespace zuri_run {
    tempo_utils::Status run_package_command(
        std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
        const std::filesystem::path &mainPackagePath,
        const std::vector<std::string> &mainArgs);
}

#endif // ZURI_RUN_RUN_PACKAGE_COMMAND_H