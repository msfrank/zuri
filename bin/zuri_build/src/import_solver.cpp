
#include <zuri_build/import_solver.h>
#include <zuri_build/build_result.h>
#include <zuri_distributor/http_package_resolver.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_tooling/package_manager.h>

zuri_build::ImportSolver::ImportSolver(std::shared_ptr<zuri_tooling::PackageManager> packageManager)
    : m_packageManager(std::move(packageManager))
{
    TU_ASSERT (m_packageManager != nullptr);
}

tempo_utils::Status
zuri_build::ImportSolver::configure()
{
    if (m_selector != nullptr)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "import solver is already configured");

    m_dcache = m_packageManager->getDcache();
    m_ucache = m_packageManager->getUcache();
    m_icache = m_packageManager->getIcache();

    zuri_distributor::HttpPackageResolverOptions resolverOptions;
    std::shared_ptr<zuri_distributor::AbstractPackageResolver> resolver;
    TU_ASSIGN_OR_RETURN (resolver, zuri_distributor::HttpPackageResolver::create(resolverOptions));

    auto selector = std::make_unique<zuri_distributor::DependencySelector>(resolver);

    zuri_distributor::PackageFetcherOptions fetcherOptions;
    auto fetcher = std::make_unique<zuri_distributor::PackageFetcher>(fetcherOptions);
    TU_RETURN_IF_NOT_OK (fetcher->configure());

    m_resolver = std::move(resolver);
    m_fetcher = std::move(fetcher);
    m_selector = std::move(selector);

    return {};
}

tempo_utils::Status
zuri_build::ImportSolver::addImport(
    const zuri_packager::PackageSpecifier &specifier,
    std::string_view shortcut)
{
    return m_selector->addDirectDependency(specifier, shortcut);
}

tempo_utils::Status
zuri_build::ImportSolver::addImport(
    const tempo_utils::Url &url,
    std::string_view shortcut)
{
    if (m_urlShortcuts.contains(url))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "import already defined for {}", url.toString());
    m_urlShortcuts[url] = shortcut;
    return {};
}


tempo_utils::Status
zuri_build::ImportSolver::installImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver)
{
    // copy url shortcuts to local and clear the member
    auto urlShortcuts = std::move(m_urlShortcuts);
    m_urlShortcuts.clear();

    // create a request for each package url
    std::vector<std::string> urlIds;
    for (const auto &entry : urlShortcuts) {
        std::string id;
        TU_ASSIGN_OR_RETURN (id, m_fetcher->requestFile(entry.first));
        urlIds.push_back(std::move(id));
    }

    // fetch all packages specified by url
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // add direct dependencies on packages downloaded from urls
    bool failed = false;
    for (auto it = m_fetcher->resultsBegin(); it != m_fetcher->resultsEnd(); it++) {
        const auto &result = it->second;
        if (result.status.isOk()) {
            auto shortcut = urlShortcuts.at(result.url);
            TU_RETURN_IF_NOT_OK (m_selector->addDirectDependency(result.path, shortcut));
        } else {
            TU_LOG_V << "failed to download " << result.url;
            failed = true;
        }
    }

    if (failed)
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "not all requested packages could be downloaded");

    // resolve all transitive dependencies and generate the install ordering
    std::vector<zuri_distributor::Selection> dependencyOrder;
    TU_ASSIGN_OR_RETURN (dependencyOrder, m_selector->calculateDependencyOrder());

    // add each missing dependency to fetcher
    int numPackagesToInstall = 0;
    for (const auto &selection : dependencyOrder) {

        // request download if the package is not present in any of the available caches
        if (!packageIsPresent(selection.specifier)) {
            TU_RETURN_IF_NOT_OK (m_fetcher->requestFile(selection.url, selection.specifier.toString()));
            numPackagesToInstall++;
        } else {
            TU_LOG_V << "ignoring " << selection.specifier.toString() << ": already installed";
        }

        // insert shortcut if specified
        if (!selection.shortcut.empty()) {
            auto origin = selection.specifier.toUrlOrigin();
            TU_RETURN_IF_NOT_OK (shortcutResolver->insertShortcut(selection.shortcut, origin));
            TU_LOG_V << "added shortcut '" << selection.shortcut << "' for " << selection.specifier.toString();
        }
    }

    if (numPackagesToInstall == 0) {
        TU_LOG_V << "all packages are installed, nothing to do";
    } else {
        TU_LOG_V << "installing " << numPackagesToInstall << " packages";
    }

    // fetch missing dependencies
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // install fetched dependencies into import package cache
    for (const auto &selection : dependencyOrder) {
        auto id = selection.specifier.toString();
        if (m_fetcher->hasResult(id)) {
            auto result = m_fetcher->getResult(id);
            TU_RETURN_IF_NOT_OK (result.status);
            TU_RETURN_IF_STATUS (m_icache->installPackage(result.path));
        }
    }

    return {};
}

bool
zuri_build::ImportSolver::packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const
{
    if (m_icache != nullptr && m_icache->containsPackage(specifier))
        return true;
    if (m_ucache != nullptr && m_ucache->containsPackage(specifier))
        return true;
    return m_dcache != nullptr && m_dcache->containsPackage(specifier);
}