/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_TOOLING_ZURI_CONFIG_H
#define ZURI_TOOLING_ZURI_CONFIG_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "import_store.h"
#include "package_store.h"
#include "target_store.h"
#include "build_tool_config.h"

namespace zuri_tooling {

    constexpr const char * const kEnvOverrideConfigName = "ZURI_OVERRIDE_CONFIG";
    constexpr const char * const kEnvOverrideVendorConfigName = "ZURI_OVERRIDE_VENDOR_CONFIG";
    constexpr const char * const kDefaultUserDirectoryName = ".zuri";

    /**
     *
     */
    class ZuriConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forSystem(
            const std::filesystem::path &distributionRootOverride = {});
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forUser(
            const std::filesystem::path &userHomeOverride = {},
            const std::filesystem::path &distributionRootOverride = {});
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forWorkspace(
            const std::filesystem::path &workspaceConfigFile,
            const std::filesystem::path &userHomeOverride = {},
            const std::filesystem::path &distributionRootOverride = {});

        std::filesystem::path getDistributionRoot() const;
        std::filesystem::path getUserRoot() const;
        std::filesystem::path getWorkspaceRoot() const;
        std::filesystem::path getWorkspaceConfigFile() const;

        std::shared_ptr<ImportStore> getImportStore() const;
        std::shared_ptr<TargetStore> getTargetStore() const;
        std::shared_ptr<PackageStore> getPackageStore() const;
        std::shared_ptr<BuildToolConfig> getBuildToolConfig() const;

    private:
        std::filesystem::path m_distributionRoot;
        std::filesystem::path m_userRoot;
        std::filesystem::path m_workspaceConfigFile;
        tempo_config::ConfigMap m_zuriMap;
        tempo_config::ConfigMap m_vendorMap;

        std::shared_ptr<ImportStore> m_importStore;
        std::shared_ptr<TargetStore> m_targetStore;
        std::shared_ptr<PackageStore> m_packageStore;
        std::shared_ptr<BuildToolConfig> m_buildToolConfig;

        ZuriConfig(
            const std::filesystem::path &distributionRoot,
            const std::filesystem::path &userRoot,
            const std::filesystem::path &workspaceConfigFile,
            const tempo_config::ConfigMap &zuriMap,
            const tempo_config::ConfigMap &vendorMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_TOOLING_ZURI_CONFIG_H
