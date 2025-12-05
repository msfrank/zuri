
#include <zuri_tooling/package_manager.h>

#include "zuri_distributor/package_cache_loader.h"
#include "zuri_tooling/tooling_result.h"

zuri_tooling::PackageManager::PackageManager(
    std::shared_ptr<ZuriConfig> zuriConfig,
    const std::filesystem::path &buildRoot)
    : m_zuriConfig(std::move(zuriConfig)),
      m_buildRoot(buildRoot)
{
    TU_ASSERT (m_zuriConfig != nullptr);
}

tempo_utils::Status
zuri_tooling::PackageManager::configure()
{
    if (m_loader != nullptr)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "package manager is already configured");

    auto packageStore = m_zuriConfig->getPackageStore();
    if (packageStore == nullptr)
        return ToolingStatus::forCondition(ToolingCondition::kToolingInvariant,
            "'packages' section is missing from config");

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

    auto userRoot = m_zuriConfig->getUserRoot();
    auto userPackagesRoot = userRoot / ZURI_PACKAGES_DIR_NAME;
    if (std::filesystem::exists(userPackagesRoot)) {
        TU_ASSIGN_OR_RETURN (m_ucache, zuri_distributor::PackageCache::openOrCreate(
            userPackagesRoot, "user"));
        packageCaches.push_back(m_ucache);
    }

    auto distributionRoot = m_zuriConfig->getDistributionRoot();
    auto distributionPackagesRoot = distributionRoot / PACKAGES_DIR_PREFIX;
    auto systemPackageCache = distributionPackagesRoot / "system";
    if (std::filesystem::exists(systemPackageCache)) {
        TU_ASSIGN_OR_RETURN (m_dcache, zuri_distributor::PackageCache::open(systemPackageCache));
        packageCaches.push_back(m_dcache);
    }

    m_tieredCache = std::make_shared<zuri_distributor::TieredPackageCache>(packageCaches);
    m_loader = std::make_shared<zuri_distributor::PackageCacheLoader>(m_tieredCache);

    return {};
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_tooling::PackageManager::getDcache() const
{
    return m_dcache;
}

std::shared_ptr<zuri_distributor::PackageCache>
zuri_tooling::PackageManager::getUcache() const
{
    return m_ucache;
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