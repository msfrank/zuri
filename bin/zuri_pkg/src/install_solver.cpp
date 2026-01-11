
#include <zuri_pkg/install_solver.h>

#include "zuri_distributor/http_package_resolver.h"
#include "zuri_pkg/pkg_result.h"

zuri_pkg::InstallSolver::InstallSolver(
    std::shared_ptr<zuri_distributor::RuntimeEnvironment> runtimeEnvironment,
    bool dryRun)
    : m_runtimeEnvironment(std::move(runtimeEnvironment)),
      m_dryRun(dryRun)
{
    TU_ASSERT (m_runtimeEnvironment != nullptr);
}

tempo_utils::Status
zuri_pkg::InstallSolver::configure()
{
    if (m_selector != nullptr)
        return PkgStatus::forCondition(PkgCondition::kPkgInvariant,
            "install solver is already configured");

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
    TU_RETURN_IF_STATUS (m_selector->addDirectDependency(id));
    return {};
}

tempo_utils::Status
zuri_pkg::InstallSolver::addPackage(const zuri_packager::PackageSpecifier &specifier)
{
    TU_RETURN_IF_STATUS (m_selector->addDirectDependency(specifier));
    return {};
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
            TU_RETURN_IF_STATUS (m_selector->addDirectDependency(result.path));
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
    int numPackagesToInstall = 0;
    for (const auto &selection : dependencyOrder) {
        if (!m_runtimeEnvironment->containsPackage(selection.specifier)) {
            TU_RETURN_IF_NOT_OK (m_fetcher->requestFile(selection.url, selection.specifier.toString()));
            numPackagesToInstall++;
        } else {
            TU_CONSOLE_OUT << "ignoring " << selection.specifier.toString() << ": already installed";
        }
    }

    if (numPackagesToInstall == 0) {
        TU_CONSOLE_OUT << "all packages are installed, nothing to do";
        return {};
    }

    TU_CONSOLE_OUT << "installing " << numPackagesToInstall << " packages";

    // fetch missing dependencies
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // install fetched dependencies into install cache
    for (const auto &selection : dependencyOrder) {
        auto id = selection.specifier.toString();
        if (m_fetcher->hasResult(id)) {
            auto result = m_fetcher->getResult(id);
            TU_RETURN_IF_NOT_OK (result.status);
            if (!m_dryRun) {
                std::filesystem::path installPath;
                TU_ASSIGN_OR_RETURN (installPath, m_runtimeEnvironment->installPackage(result.path));
                TU_LOG_V << "installed " << selection.specifier.toString() << " in " << installPath;
            } else {
                TU_CONSOLE_OUT << "DRY RUN: install package " << result.path;
            }
        }
    }

    return {};
}