#ifndef ZURI_BUILD_IMPORT_RESOLVER_H
#define ZURI_BUILD_IMPORT_RESOLVER_H

#include <zuri_distributor/abstract_package_resolver.h>
#include <zuri_distributor/package_cache.h>

struct ResolverEntry {
    zuri_packager::PackageId id;
    std::string requirement;
    std::shared_ptr<zuri_packager::PackageReader> reader;
};

class ImportResolver {
public:
    ImportResolver(
        const tempo_config::ConfigMap &resolverConfig,
        std::shared_ptr<zuri_distributor::PackageCache> importPackageCache);

    tempo_utils::Status configure();

    tempo_utils::Status addDependency(const zuri_packager::PackageDependency &dependency);

    tempo_utils::Status resolve();

private:
    tempo_config::ConfigMap m_resolverConfig;
    std::shared_ptr<zuri_distributor::PackageCache> m_importPackageCache;

    std::vector<std::shared_ptr<zuri_distributor::AbstractPackageResolver>> m_resolvers;
    absl::flat_hash_map<zuri_packager::PackageId, zuri_packager::RequirementsList> m_dependencies;
};

#endif // ZURI_BUILD_IMPORT_RESOLVER_H
