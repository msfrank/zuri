// #ifndef ZURI_TOOLING_PACKAGE_MANAGER_H
// #define ZURI_TOOLING_PACKAGE_MANAGER_H
//
// #include <lyric_runtime/abstract_loader.h>
// #include <zuri_distributor/package_cache.h>
// #include <zuri_distributor/package_cache_loader.h>
// #include <zuri_distributor/tiered_package_cache.h>
//
// #include "environment_config.h"
//
// namespace zuri_tooling {
//
//     class PackageManager {
//     public:
//         PackageManager(
//             std::shared_ptr<EnvironmentConfig> environmentConfig,
//             const std::filesystem::path &buildRoot = {});
//
//         tempo_utils::Status configure();
//
//         std::shared_ptr<zuri_distributor::PackageCache> getEcache() const;
//         std::shared_ptr<zuri_distributor::PackageCache> getIcache() const;
//         std::shared_ptr<zuri_distributor::PackageCache> getTcache() const;
//         std::shared_ptr<zuri_distributor::TieredPackageCache> getTieredCache() const;
//         std::shared_ptr<lyric_runtime::AbstractLoader> getLoader() const;
//
//     private:
//         std::shared_ptr<EnvironmentConfig> m_environmentConfig;
//         std::filesystem::path m_buildRoot;
//
//         std::shared_ptr<zuri_distributor::PackageCache> m_ecache;
//         std::shared_ptr<zuri_distributor::PackageCache> m_icache;
//         std::shared_ptr<zuri_distributor::PackageCache> m_tcache;
//         std::shared_ptr<zuri_distributor::TieredPackageCache> m_tieredCache;
//         std::shared_ptr<zuri_distributor::PackageCacheLoader> m_loader;
//     };
// }
//
// #endif // ZURI_TOOLING_PACKAGE_MANAGER_H