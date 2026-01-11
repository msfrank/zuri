#ifndef ZURI_RUN_RUN_INTERACTIVE_COMMAND_H
#define ZURI_RUN_RUN_INTERACTIVE_COMMAND_H

#include <tempo_utils/status.h>
#include <zuri_tooling/environment_config.h>
#include <zuri_tooling/build_tool_config.h>

namespace zuri_run {
    tempo_utils::Status run_interactive_command(
        std::shared_ptr<zuri_tooling::EnvironmentConfig> environmentConfig,
        std::shared_ptr<zuri_tooling::BuildToolConfig> buildToolConfig,
        const std::vector<std::string> &mainArgs);
}

#endif // ZURI_RUN_RUN_INTERACTIVE_COMMAND_H