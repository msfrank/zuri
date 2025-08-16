
#include <zuri_build/import_resolver.h>
#include <zuri_build/build_result.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_distributor/static_package_resolver.h>
#include <zuri_tooling/package_manager.h>

zuri_build::ImportResolver::ImportResolver(std::shared_ptr<zuri_tooling::PackageManager> packageManager)
    : m_packageManager(std::move(packageManager))
{
    TU_ASSERT (m_packageManager != nullptr);
}

tempo_utils::Status
zuri_build::ImportResolver::configure()
{
    if (m_selector != nullptr)
        return zuri_build::BuildStatus::forCondition(zuri_build::BuildCondition::kBuildInvariant,
            "import resolver is already configured");

    m_dcache = m_packageManager->getDcache();
    m_ucache = m_packageManager->getUcache();
    m_icache = m_packageManager->getIcache();

    //auto httpResolver = std::make_shared<zuri_distributor::HttpPackageResolver>();
    std::shared_ptr<zuri_distributor::AbstractPackageResolver> resolver;
    TU_ASSIGN_OR_RETURN (resolver, zuri_distributor::StaticPackageResolver::create({}));

    auto selector = std::make_unique<zuri_distributor::DependencySelector>(resolver);

    m_resolver = std::move(resolver);
    m_selector = std::move(selector);

    return {};
}

tempo_utils::Status
zuri_build::ImportResolver::addRequirement(
    const zuri_packager::PackageSpecifier &requirement,
    std::string_view shortcut)
{
    return m_selector->addDirectDependency(requirement, shortcut);
}

struct PendingRequirement {
    zuri_packager::PackageId targetId;
};

tempo_utils::Status
zuri_build::ImportResolver::resolveImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver)
{
    // resolve all transitive dependencies and generate the dependency ordering
    std::vector<zuri_distributor::Selection> dependencyOrder;
    TU_ASSIGN_OR_RETURN (dependencyOrder, m_selector->calculateDependencyOrder());

    // create and configure fetcher
    zuri_distributor::PackageFetcher fetcher;
    TU_RETURN_IF_NOT_OK (fetcher.configure());

    // add each missing dependency to fetcher
    for (const auto &selection : dependencyOrder) {
        if (!packageIsPresent(selection.specifier)) {
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
            TU_RETURN_IF_STATUS (m_icache->installPackage(result.path));
        }

        // insert shortcut if specified
        if (!selection.shortcut.empty()) {
            auto origin = selection.specifier.toUrlOrigin();
            TU_RETURN_IF_NOT_OK (shortcutResolver->insertShortcut(selection.shortcut, origin));
        }
    }

    return {};
}

bool
zuri_build::ImportResolver::packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const
{
    if (m_icache->containsPackage(specifier))
        return true;
    if (m_ucache->containsPackage(specifier))
        return true;
    if (m_dcache->containsPackage(specifier))
        return true;
    return false;
}