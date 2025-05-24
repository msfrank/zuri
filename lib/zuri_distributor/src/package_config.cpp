
#include <tempo_config/parse_config.h>
#include <zuri_distributor/package_config.h>

zuri_distributor::PackageConfig::PackageConfig(
    const std::filesystem::path &packageRoot,
    tempo_config::ConfigMap &packageConfig)
    : m_packageRoot(packageRoot),
      m_packageConfig(packageConfig)
{
    TU_ASSERT (!m_packageRoot.empty());
}

std::filesystem::path
zuri_distributor::PackageConfig::getPackageRoot() const
{
    return m_packageRoot;
}

tempo_config::ConfigMap
zuri_distributor::PackageConfig::getPackageConfig() const
{
    return m_packageConfig;
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::PackageConfig>>
zuri_distributor::PackageConfig::load(
    const std::filesystem::path &packageConfigFilePath,
    const PackageConfigOptions &options)
{
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, tempo_config::read_config_map_file(packageConfigFilePath));

    auto packageRoot = packageConfigFilePath.parent_path();

    return std::shared_ptr<PackageConfig>(new PackageConfig(packageRoot, packageConfig));
}
