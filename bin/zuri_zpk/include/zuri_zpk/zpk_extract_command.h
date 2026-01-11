#ifndef ZURI_ZPK_ZPK_EXTRACT_COMMAND_H
#define ZURI_ZPK_ZPK_EXTRACT_COMMAND_H

#include <tempo_command/command_tokenizer.h>
#include <tempo_utils/status.h>
#include <zuri_tooling/core_config.h>

namespace zuri_zpk {
    tempo_utils::Status zpk_extract_command(
        std::shared_ptr<zuri_tooling::CoreConfig> coreConfig,
        tempo_command::TokenVector &tokens);
}

#endif // ZURI_ZPK_ZPK_EXTRACT_COMMAND_H