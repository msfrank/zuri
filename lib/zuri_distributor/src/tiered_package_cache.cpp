
#include <absl/strings/str_split.h>

#include <tempo_config/config_utils.h>
#include <tempo_utils/tempdir_maker.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_distributor/tiered_package_cache.h>

zuri_distributor::TieredPackageCache::TieredPackageCache(
    const std::vector<std::shared_ptr<AbstractReadonlyPackageCache>> &packageCaches)
    : m_packageCaches(packageCaches)
{
    TU_ASSERT (!m_packageCaches.empty());
}

bool
zuri_distributor::TieredPackageCache::containsPackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return false;
    for (const auto &packageCache : m_packageCaches) {
        if (packageCache->containsPackage(specifier))
            return true;
    }
    return false;
}

tempo_utils::Result<Option<tempo_config::ConfigMap>>
zuri_distributor::TieredPackageCache::describePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    Option<std::filesystem::path> pathOption;
    TU_ASSIGN_OR_RETURN (pathOption, resolvePackage(specifier));
    if (pathOption.isEmpty())
        return Option<tempo_config::ConfigMap>{};
    tempo_config::ConfigMap packageConfig;
    TU_ASSIGN_OR_RETURN (packageConfig, tempo_config::read_config_map_file(
        pathOption.getValue() / "package.config"));
    return Option(packageConfig);
}

tempo_utils::Result<Option<std::filesystem::path>>
zuri_distributor::TieredPackageCache::resolvePackage(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!specifier.isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "invalid package specifier");

    for (const auto &packageCache : m_packageCaches) {
        Option<std::filesystem::path> packagePathOption;
        TU_ASSIGN_OR_RETURN (packagePathOption, packageCache->resolvePackage(specifier));
        if (packagePathOption.hasValue())
            return packagePathOption;
    }

    return {};
}

tempo_utils::Result<std::shared_ptr<zuri_distributor::TieredPackageCache>>
zuri_distributor::TieredPackageCache::create(
    const std::vector<std::filesystem::path> &packageCacheDirectories)
{
    std::vector<std::shared_ptr<AbstractReadonlyPackageCache>> packageCaches;

    for (const auto &packageCacheDirectory : packageCacheDirectories) {
        std::shared_ptr<PackageCache> packageCache;
        TU_ASSIGN_OR_RETURN (packageCache, PackageCache::open(packageCacheDirectory));
        packageCaches.push_back(std::move(packageCache));
    }

    return std::make_shared<TieredPackageCache>(packageCaches);
}