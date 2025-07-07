#ifndef ZURI_BUILD_IMPORT_RESOLVER_H
#define ZURI_BUILD_IMPORT_RESOLVER_H

#include <zuri_distributor/abstract_package_resolver.h>
#include <zuri_distributor/package_cache.h>

class ImportResolver {
public:
    ImportResolver(
        const tempo_config::ConfigMap &resolverConfig,
        std::shared_ptr<zuri_distributor::PackageCache> importPackageCache);

    tempo_utils::Status configure();

private:
    tempo_config::ConfigMap m_resolverConfig;
    std::shared_ptr<zuri_distributor::PackageCache> m_importPackageCache;

    std::shared_ptr<zuri_distributor::AbstractPackageResolver> m_packageResolver;
};

#endif // ZURI_BUILD_IMPORT_RESOLVER_H
