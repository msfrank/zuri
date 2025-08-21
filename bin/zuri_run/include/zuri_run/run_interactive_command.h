#ifndef ZURI_RUN_RUN_INTERACTIVE_COMMAND_H
#define ZURI_RUN_RUN_INTERACTIVE_COMMAND_H

#include <tempo_utils/status.h>
#include <zuri_tooling/zuri_config.h>

namespace zuri_run {
    tempo_utils::Status run_interactive_command(
        std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
        const std::string &sessionId,
        const std::vector<std::string> &mainArgs);
}

#endif // ZURI_RUN_RUN_INTERACTIVE_COMMAND_H