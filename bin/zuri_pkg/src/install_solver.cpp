
#include <zuri_pkg/install_solver.h>

#include "zuri_distributor/http_package_resolver.h"
#include "zuri_pkg/pkg_result.h"

zuri_pkg::InstallSolver::InstallSolver(
    std::shared_ptr<zuri_tooling::PackageManager> packageManager,
    bool systemInstall,
    bool dryRun)
    : m_packageManager(std::move(packageManager)),
      m_systemInstall(systemInstall),
      m_dryRun(dryRun)
{
    TU_ASSERT (m_packageManager != nullptr);
}

tempo_utils::Status
zuri_pkg::InstallSolver::configure()
{
    if (m_selector != nullptr)
        return PkgStatus::forCondition(PkgCondition::kPkgInvariant,
            "install solver is already configured");

    m_dcache = m_packageManager->getDcache();
    m_ucache = m_packageManager->getUcache();

    m_installCache = m_systemInstall? m_dcache : m_ucache;
    if (m_installCache == nullptr)
        return PkgStatus::forCondition(PkgCondition::kPkgInvariant,
            "install cache is not available");

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
zuri_pkg::InstallSolver::addPackage(const zuri_packager::PackageId &id)
{
    return m_selector->addDirectDependency(id);
}

tempo_utils::Status
zuri_pkg::InstallSolver::addPackage(const zuri_packager::PackageSpecifier &specifier)
{
    return m_selector->addDirectDependency(specifier);
}

tempo_utils::Status
zuri_pkg::InstallSolver::addPackage(const tempo_utils::Url &url)
{
    m_packageUrls.insert(url);
    return {};
}

tempo_utils::Status
zuri_pkg::InstallSolver::installPackages()
{
    // create a request for each package url
    std::vector<std::string> urlIds;
    for (const auto &url : m_packageUrls) {
        std::string id;
        TU_ASSIGN_OR_RETURN (id, m_fetcher->requestFile(url));
        urlIds.push_back(std::move(id));
    }
    m_packageUrls.clear();

    // fetch all packages specified by url
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // add direct dependencies on packages downloaded from urls
    bool failed = false;
    for (auto it = m_fetcher->resultsBegin(); it != m_fetcher->resultsEnd(); it++) {
        const auto &result = it->second;
        if (result.status.isOk()) {
            TU_RETURN_IF_NOT_OK (m_selector->addDirectDependency(result.path));
        } else {
            TU_LOG_V << "failed to download " << result.url;
            failed = true;
        }
    }
    if (failed)
        return PkgStatus::forCondition(PkgCondition::kPkgInvariant,
            "not all requested packages could be downloaded");

    // resolve all transitive dependencies and generate the install ordering
    std::vector<zuri_distributor::Selection> dependencyOrder;
    TU_ASSIGN_OR_RETURN (dependencyOrder, m_selector->calculateDependencyOrder());

    // add each missing dependency to fetcher
    for (const auto &selection : dependencyOrder) {
        if (!packageIsPresent(selection.specifier)) {
            TU_RETURN_IF_NOT_OK (m_fetcher->requestFile(selection.url, selection.specifier.toString()));
        }
    }

    // fetch missing dependencies
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // install fetched dependencies into import package cache
    for (const auto &selection : dependencyOrder) {
        auto id = selection.specifier.toString();
        if (m_fetcher->hasResult(id)) {
            auto result = m_fetcher->getResult(id);
            TU_RETURN_IF_NOT_OK (result.status);
            if (!m_dryRun) {
                //TU_RETURN_IF_STATUS (m_installCache->installPackage(result.path));
                TU_CONSOLE_OUT << "install package " << result.path;
            } else {
                TU_CONSOLE_OUT << "DRY RUN: install package " << result.path;
            }
        }
    }

    return {};
}

bool
zuri_pkg::InstallSolver::packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const
{
    if (!m_systemInstall) {
        if (m_ucache->containsPackage(specifier))
            return true;
    }
    return m_dcache->containsPackage(specifier);
}