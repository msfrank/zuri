/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_TOOLING_ZURI_WORKSPACE_H
#define ZURI_TOOLING_ZURI_WORKSPACE_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "import_store.h"
#include "target_store.h"

namespace zuri_tooling {

    constexpr const char * const kEnvOverrideConfigName = "ZURI_BUILD_OVERRIDE_CONFIG";

    constexpr const char * const kEnvOverrideVendorConfigName = "ZURI_BUILD_OVERRIDE_VENDOR_CONFIG";

    /**
     *
     */
    class ZuriWorkspace {

    public:
        static tempo_utils::Result<std::shared_ptr<ZuriWorkspace>> find(
            const std::filesystem::path &searchPathStart,
            const std::filesystem::path &distributionRoot);

        static tempo_utils::Result<std::shared_ptr<ZuriWorkspace>> load(
            const std::filesystem::path &workspaceConfigFile,
            const std::filesystem::path &distributionRoot);

        std::shared_ptr<ImportStore> getImportStore() const;
        std::shared_ptr<TargetStore> getTargetStore() const;

    private:
        tempo_config::ConfigMap m_workspaceMap;
        tempo_config::ConfigMap m_vendorMap;

        std::shared_ptr<ImportStore> m_importStore;
        std::shared_ptr<TargetStore> m_targetStore;

        ZuriWorkspace(
            const tempo_config::ConfigMap &workspaceMap,
            const tempo_config::ConfigMap &vendorMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_TOOLING_ZURI_WORKSPACE_H
