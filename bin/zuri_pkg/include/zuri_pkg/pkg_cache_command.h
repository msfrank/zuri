#ifndef ZURI_PKG_PKG_CACHE_COMMAND_H
#define ZURI_PKG_PKG_CACHE_COMMAND_H

#include <tempo_command/command_tokenizer.h>
#include <tempo_utils/status.h>
#include <zuri_tooling/zuri_config.h>

namespace zuri_pkg {
    tempo_utils::Status pkg_cache_command(
        std::shared_ptr<zuri_tooling::ZuriConfig> zuriConfig,
        bool manageSystem,
        tempo_command::TokenVector &tokens);
}

#endif // ZURI_PKG_PKG_CACHE_COMMAND_H