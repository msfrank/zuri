
#include <zuri_distributor/package_cache_loader.h>
#include <zuri_tooling/package_manager.h>
#include <zuri_tooling/tooling_result.h>

zuri_tooling::PackageManager::PackageManager(
    std::shared_ptr<EnvironmentConfig> environmentConfig,
    const std::filesystem::path &buildRoot)
    : m_environmentConfig(std::move(environmentConfig)),
      m_buildRoot(buildRoot)
{
    TU_ASSERT (m_environmentConfig != nullptr);
}

tempo_utils::Status
zuri_tooling::PackageManager::configure()
{
    if (m_loader != nullptr)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "package manager is already configured");

    std::vector<std::shared_ptr<zuri_distributor::AbstractReadonlyPackageCache>> packageCaches;

    if (!m_buildRoot.empty()) {
        if (!std::filesystem::exists(m_buildRoot))
            return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
                "build root '{}' does not exist", m_buildRoot.string());
        TU_ASSIGN_OR_RETURN (m_tcache, zuri_distributor::PackageCache::openOrCreate(
            m_buildRoot, "targets"));
        packageCaches.push_back(m_tcache);
        TU_ASSIGN_OR_RETURN (m_icache, zuri_distributor::PackageCache::openOrCreate(
            m_buildRoot, "imports"));
        packageCaches.push_back(m_icache);
    }

    auto environment = m_environmentConfig->getEnvironment();
    auto envPackagesDirectory = environment.getPackagesDirectory();
    TU_ASSIGN_OR_RETURN (m_ecache, zuri_distributor::PackageCache::open(envPackagesDirectory));
    packageCaches.push_back(m_ecache);

    m_tieredCache = std::make_shared<zuri_distributor::TieredPackageCache>(packageCaches);
    m_loader = std::make_shared<zuri_distributor::PackageCacheLoader>(m_tieredCache);

    return {};
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_tooling::PackageManager::getEcache() const
{
    return m_ecache;
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_tooling::PackageManager::getIcache() const
{
    return m_icache;
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_tooling::PackageManager::getTcache() const
{
    return m_tcache;
}

std::shared_ptr<zuri_distributor::TieredPackageCache>
zuri_tooling::PackageManager::getTieredCache() const
{
    return m_tieredCache;
}

std::shared_ptr<lyric_runtime::AbstractLoader>
zuri_tooling::PackageManager::getLoader() const
{
    return m_loader;
}