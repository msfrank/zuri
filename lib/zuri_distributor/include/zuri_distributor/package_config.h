#ifndef ZURI_DISTRIBUTOR_PACKAGE_CONFIG_H
#define ZURI_DISTRIBUTOR_PACKAGE_CONFIG_H

#include <filesystem>

#include <tempo_config/config_types.h>
#include <tempo_utils/result.h>

namespace zuri_distributor {

    struct PackageConfigOptions {
    };

    class PackageConfig {
    public:
        static tempo_utils::Result<std::shared_ptr<PackageConfig>> load(
            const std::filesystem::path &packageConfigFilePath,
            const PackageConfigOptions &options);

        std::filesystem::path getPackageRoot() const;

        tempo_config::ConfigMap getPackageConfig() const;

    private:
        std::filesystem::path m_packageRoot;
        tempo_config::ConfigMap m_packageConfig;

        PackageConfig(
            const std::filesystem::path &packageRoot,
            tempo_config::ConfigMap &packageConfig);
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_CONFIG_H
