
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

    std::vector<std::shared_ptr<lyric_runtime::AbstractLoader>> loaderChain;

    auto distributionRoot = m_zuriConfig->getDistributionRoot();
    auto distributionPackagesRoot = distributionRoot / "lib" / "zuri-packages-1";
    auto systemPackageCache = distributionPackagesRoot / "system";
    if (std::filesystem::exists(systemPackageCache)) {
        TU_ASSIGN_OR_RETURN (m_dcache, zuri_distributor::PackageCache::openOrCreate(
            distributionPackagesRoot, "system"));
        loaderChain.push_back(std::make_shared<zuri_distributor::PackageCacheLoader>(m_dcache));
    }

    auto userRoot = m_zuriConfig->getUserRoot();
    auto userPackagesRoot = userRoot / "zuri-packages-1";
    auto userPackageCache = userPackagesRoot / "user";
    if (std::filesystem::exists(userPackageCache)) {
        TU_ASSIGN_OR_RETURN (m_ucache, zuri_distributor::PackageCache::openOrCreate(
            userPackagesRoot, "user"));
        loaderChain.push_back(std::make_shared<zuri_distributor::PackageCacheLoader>(m_ucache));
    }

    if (!m_buildRoot.empty()) {
        auto importsPackageCache = m_buildRoot / "imports";
        TU_ASSIGN_OR_RETURN (m_icache, zuri_distributor::PackageCache::openOrCreate(
            m_buildRoot, "imports"));
        loaderChain.push_back(std::make_shared<zuri_distributor::PackageCacheLoader>(m_icache));
        auto targetsPackageCache = m_buildRoot / "targets";
        TU_ASSIGN_OR_RETURN (m_tcache, zuri_distributor::PackageCache::openOrCreate(
            m_buildRoot, "targets"));
        loaderChain.push_back(std::make_shared<zuri_distributor::PackageCacheLoader>(m_tcache));
    }

    m_loader = std::make_shared<lyric_runtime::ChainLoader>(loaderChain);

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

std::shared_ptr<lyric_runtime::AbstractLoader>
zuri_tooling::PackageManager::getLoader() const
{
    return m_loader;
}