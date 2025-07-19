#ifndef ZURI_BUILD_IMPORT_RESOLVER_H
#define ZURI_BUILD_IMPORT_RESOLVER_H

#include <lyric_importer/shortcut_resolver.h>
#include <zuri_distributor/abstract_package_resolver.h>
#include <zuri_distributor/package_cache.h>

#include "zuri_distributor/dependency_selector.h"

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

    tempo_utils::Status addRequirement(
        const zuri_packager::PackageSpecifier &requirement,
        std::string_view shortcut);

    tempo_utils::Status resolveImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver);

private:
    tempo_config::ConfigMap m_resolverConfig;
    std::shared_ptr<zuri_distributor::PackageCache> m_importPackageCache;
    std::shared_ptr<zuri_distributor::AbstractPackageResolver> m_resolver;
    std::unique_ptr<zuri_distributor::DependencySelector> m_selector;
};

#endif // ZURI_BUILD_IMPORT_RESOLVER_H
