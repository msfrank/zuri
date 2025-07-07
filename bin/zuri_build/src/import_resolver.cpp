
#include <zuri_build/import_installer.h>

ImportResolver::ImportResolver(
    const tempo_config::ConfigMap &resolverConfig,
    std::shared_ptr<zuri_distributor::PackageCache> importPackageCache)
    : m_resolverConfig(resolverConfig),
      m_importPackageCache(std::move(importPackageCache))
{
    TU_ASSERT (m_importPackageCache != nullptr);
}

tempo_utils::Status
ImportResolver::configure()
{
    return {};
}
