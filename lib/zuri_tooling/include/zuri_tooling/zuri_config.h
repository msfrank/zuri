/* SPDX-License-Identifier: AGPL-3.0-or-later */

#ifndef ZURI_TOOLING_ZURI_CONFIG_H
#define ZURI_TOOLING_ZURI_CONFIG_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

#include "build_tool_config.h"
#include "distribution.h"
#include "home.h"
#include "import_store.h"
#include "package_store.h"
#include "target_store.h"

namespace zuri_tooling {

    constexpr const char * const kEnvOverrideConfigName = "ZURI_OVERRIDE_CONFIG";
    constexpr const char * const kEnvOverrideVendorConfigName = "ZURI_OVERRIDE_VENDOR_CONFIG";
    constexpr const char * const kEnvHomeName = "ZURI_HOME";

    /**
     *
     */
    class ZuriConfig {

    public:
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forSystem(
            const Distribution &distribution);
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forUser(
            const Home &home,
            const Distribution &distribution);
        static tempo_utils::Result<std::shared_ptr<ZuriConfig>> forWorkspace(
            const std::filesystem::path &workspaceConfigFile,
            const Home &home,
            const Distribution &distribution);

        Distribution getDistribution() const;
        Home getHome() const;

        std::filesystem::path getWorkspaceRoot() const;
        std::filesystem::path getWorkspaceConfigFile() const;

        std::shared_ptr<ImportStore> getImportStore() const;
        std::shared_ptr<TargetStore> getTargetStore() const;
        std::shared_ptr<PackageStore> getPackageStore() const;
        std::shared_ptr<BuildToolConfig> getBuildToolConfig() const;

    private:
        Distribution m_distribution;
        Home m_home;
        std::filesystem::path m_workspaceConfigFile;
        tempo_config::ConfigMap m_zuriMap;
        tempo_config::ConfigMap m_vendorMap;

        std::shared_ptr<ImportStore> m_importStore;
        std::shared_ptr<TargetStore> m_targetStore;
        std::shared_ptr<PackageStore> m_packageStore;
        std::shared_ptr<BuildToolConfig> m_buildToolConfig;

        ZuriConfig(
            const Distribution &distribution,
            const Home &home,
            const std::filesystem::path &workspaceConfigFile,
            const tempo_config::ConfigMap &zuriMap,
            const tempo_config::ConfigMap &vendorMap);

        tempo_utils::Status configure();
    };
}

#endif // ZURI_TOOLING_ZURI_CONFIG_H
