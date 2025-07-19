
#include <zuri_build/import_resolver.h>
#include <zuri_build/program_status.h>
#include <zuri_distributor/static_package_resolver.h>

#include "zuri_distributor/package_fetcher.h"

ImportResolver::ImportResolver(
    const tempo_config::ConfigMap &resolverConfig,
    std::shared_ptr<zuri_distributor::PackageCache> importPackageCache)
    : m_resolverConfig(resolverConfig),
      m_importPackageCache(std::move(importPackageCache))
{
    TU_ASSERT (m_importPackageCache != nullptr);
}

tempo_utils::Status
ImportResolver::configure()
{
    if (m_selector != nullptr)
        return ProgramStatus::forCondition(ProgramCondition::ProgramInvariant,
            "import resolver is already configured");

    //auto httpResolver = std::make_shared<zuri_distributor::HttpPackageResolver>();
    std::shared_ptr<zuri_distributor::AbstractPackageResolver> resolver;
    TU_ASSIGN_OR_RETURN (resolver, zuri_distributor::StaticPackageResolver::create({}));

    auto selector = std::make_unique<zuri_distributor::DependencySelector>(resolver);

    m_resolver = std::move(resolver);
    m_selector = std::move(selector);

    return {};
}

tempo_utils::Status
ImportResolver::addRequirement(
    const zuri_packager::PackageSpecifier &requirement,
    std::string_view shortcut)
{
    return m_selector->addDirectDependency(requirement, shortcut);
}

struct PendingRequirement {
    zuri_packager::PackageId targetId;
};

tempo_utils::Status
ImportResolver::resolveImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver)
{
    // resolve all transitive dependencies and generate the dependency ordering
    std::vector<zuri_distributor::Selection> dependencyOrder;
    TU_ASSIGN_OR_RETURN (dependencyOrder, m_selector->calculateDependencyOrder());

    // create and configure fetcher
    zuri_distributor::PackageFetcherOptions options;
    zuri_distributor::PackageFetcher fetcher(m_resolver, options);
    TU_RETURN_IF_NOT_OK (fetcher.configure());

    // add each missing dependency to fetcher
    for (const auto &selection : dependencyOrder) {
        if (!m_importPackageCache->containsPackage(selection.specifier)) {
            TU_RETURN_IF_NOT_OK (fetcher.addPackage(selection.specifier, selection.url));
        }
    }

    // fetch missing dependencies
    TU_RETURN_IF_NOT_OK (fetcher.fetchPackages());

    // install fetched dependencies into import package cache
    for (const auto &selection : dependencyOrder) {
        if (fetcher.hasResult(selection.specifier)) {
            auto result = fetcher.getResult(selection.specifier);
            TU_RETURN_IF_NOT_OK (result.status);
            m_importPackageCache->installPackage(result.path);
        }

        // insert shortcut if specified
        if (!selection.shortcut.empty()) {
            auto origin = selection.specifier.toUrlOrigin();
            TU_RETURN_IF_NOT_OK (shortcutResolver->insertShortcut(selection.shortcut, origin));
        }
    }

    return {};
}