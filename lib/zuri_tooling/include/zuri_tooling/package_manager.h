#ifndef ZURI_TOOLING_PACKAGE_MANAGER_H
#define ZURI_TOOLING_PACKAGE_MANAGER_H

#include <lyric_runtime/abstract_loader.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_distributor/package_cache_loader.h>
#include <zuri_distributor/tiered_package_cache.h>

#include "zuri_config.h"

namespace zuri_tooling {

    class PackageManager {
    public:
        PackageManager(std::shared_ptr<ZuriConfig> zuriConfig, const std::filesystem::path &buildRoot = {});

        tempo_utils::Status configure();

        std::shared_ptr<zuri_distributor::PackageCache> getDcache() const;
        std::shared_ptr<zuri_distributor::PackageCache> getUcache() const;
        std::shared_ptr<zuri_distributor::PackageCache> getIcache() const;
        std::shared_ptr<zuri_distributor::PackageCache> getTcache() const;
        std::shared_ptr<zuri_distributor::TieredPackageCache> getTieredCache() const;
        std::shared_ptr<lyric_runtime::AbstractLoader> getLoader() const;

    private:
        std::shared_ptr<ZuriConfig> m_zuriConfig;
        std::filesystem::path m_buildRoot;

        std::shared_ptr<zuri_distributor::PackageCache> m_dcache;
        std::shared_ptr<zuri_distributor::PackageCache> m_ucache;
        std::shared_ptr<zuri_distributor::PackageCache> m_icache;
        std::shared_ptr<zuri_distributor::PackageCache> m_tcache;
        std::shared_ptr<zuri_distributor::TieredPackageCache> m_tieredCache;
        std::shared_ptr<zuri_distributor::PackageCacheLoader> m_loader;
    };
}

#endif // ZURI_TOOLING_PACKAGE_MANAGER_H