#ifndef ZURI_PROJECT_PROJECT_ADD_COMMAND_H
#define ZURI_PROJECT_PROJECT_ADD_COMMAND_H

#include <tempo_command/command_tokenizer.h>
#include <tempo_utils/status.h>
#include <zuri_tooling/core_config.h>

namespace zuri_project {

    tempo_utils::Status project_add_command(
        std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
        tempo_command::TokenVector &tokens);
}

#endif // ZURI_PROJECT_PROJECT_ADD_COMMAND_H