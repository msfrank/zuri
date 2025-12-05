#ifndef ZURI_DISTRIBUTOR_TIERED_PACKAGE_CACHE_H
#define ZURI_DISTRIBUTOR_TIERED_PACKAGE_CACHE_H

#include <tempo_utils/result.h>
#include <zuri_packager/package_reader.h>

#include "abstract_readonly_package_cache.h"

namespace zuri_distributor {

    /**
     * TieredPackageCache is an implementation of AbstractReadonlyPackageCache which acts as
     * a facade for a list of package caches. When resolving a package the TieredPackageCache
     * consults each package cache in order, returning the first package found.
     */
    class TieredPackageCache : public AbstractReadonlyPackageCache {
    public:
        explicit TieredPackageCache(
            const std::vector<std::shared_ptr<AbstractReadonlyPackageCache>> &packageCaches);

        static tempo_utils::Result<std::shared_ptr<TieredPackageCache>> create(
            const std::vector<std::filesystem::path> &packageCacheDirectories);

        bool containsPackage(const zuri_packager::PackageSpecifier &specifier) const override;

        tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageSpecifier &specifier) const override;

    private:
        std::vector<std::shared_ptr<AbstractReadonlyPackageCache>> m_packageCaches;
    };
}

#endif // ZURI_DISTRIBUTOR_TIERED_PACKAGE_CACHE_H