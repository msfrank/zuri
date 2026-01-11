
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

    // m_dcache = m_packageManager->getDcache();
    // m_ucache = m_packageManager->getUcache();
    // m_icache = m_packageManager->getIcache();

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
    const zuri_packager::PackageId &importId,
    std::shared_ptr<const zuri_tooling::ImportEntry> importEntry)
{
    switch (importEntry->type) {
        case zuri_tooling::ImportEntryType::Version: {
            zuri_packager::PackageSpecifier specifier(importId, importEntry->version);
            TU_RETURN_IF_STATUS (m_selector->addDirectDependency(specifier, importId.toString()));
            return {};
        }
        case zuri_tooling::ImportEntryType::Path: {
            TU_RETURN_IF_STATUS (m_selector->addDirectDependency(importEntry->path, importId.toString()));
            return {};
        }
        default:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "cannot import '{}'; invalid import type", importId.toString());
    }
}

tempo_utils::Status
zuri_build::ImportSolver::addTarget(
    std::string_view targetName,
    std::shared_ptr<const zuri_tooling::TargetEntry> targetEntry)
{
    if (m_targetUrls.contains(targetName))
        return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
            "import already defined for target '{}'", targetName);
    switch (targetEntry->type) {
        case zuri_tooling::TargetEntryType::Package: {
            auto package = std::get<zuri_tooling::TargetEntry::Package>(targetEntry->target);
            if (!package.url.isAbsolute())
                return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                    "cannot import target '{}'; package must refer to an absolute url", targetName);
            m_targetUrls[targetName] = package.url;
            break;
        }
        default:
            return BuildStatus::forCondition(BuildCondition::kBuildInvariant,
                "target '{}' cannot be imported", targetName);
    }
    return {};
}


tempo_utils::Result<absl::flat_hash_map<std::string,tempo_utils::Url>>
zuri_build::ImportSolver::installImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver)
{
    absl::flat_hash_map<std::string,std::string> fetcherIdTargets;
    absl::flat_hash_map<std::string,std::string> selectionIdTargets;
    absl::flat_hash_map<std::string,tempo_utils::Url> targetBases;

    // create a request for each package url
    for (const auto &entry : m_targetUrls) {
        std::string id;
        TU_ASSIGN_OR_RETURN (id, m_fetcher->requestFile(entry.second));
        fetcherIdTargets[id] = entry.first;
        TU_LOG_V << "fetcher id:" << id << " -> target:" << entry.first;
    }
    m_targetUrls.clear();

    // fetch all packages specified by url
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    // add direct dependencies on packages downloaded from urls
    bool failed = false;
    for (auto it = m_fetcher->resultsBegin(); it != m_fetcher->resultsEnd(); it++) {
        const auto &result = it->second;
        if (result.status.isOk()) {
            std::string selectionId;
            TU_ASSIGN_OR_RETURN (selectionId, m_selector->addDirectDependency(result.path));
            const auto &target = fetcherIdTargets.at(result.id);
            selectionIdTargets[selectionId] = target;
            TU_LOG_V << "selection id:" << selectionId << " -> target:" << target;
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
            auto base = selection.specifier.toUrl();
            TU_RETURN_IF_NOT_OK (shortcutResolver->insertShortcut(selection.shortcut, base));
            TU_LOG_V << "added shortcut '" << selection.shortcut << "' for " << selection.specifier.toString();
        } else {
            // otherwise if selection is a package target then insert the origin
            TU_LOG_V << "selection id:" << selection.id;
            auto entry = selectionIdTargets.find(selection.id);
            if (entry != selectionIdTargets.cend()) {
                targetBases[entry->second] = selection.specifier.toUrl();
                TU_LOG_V << "target:" << entry->second << " -> origin:" << selection.specifier.toUrl().toString();
            }
        }
    }

    if (numPackagesToInstall == 0) {
        TU_LOG_V << "all packages are installed, nothing to do";
    } else {
        TU_LOG_V << "installing " << numPackagesToInstall << " packages";
    }

    // fetch missing dependencies
    TU_RETURN_IF_NOT_OK (m_fetcher->fetchFiles());

    auto importPackageCache = m_packageManager->getIcache();

    // install fetched dependencies into import package cache
    for (const auto &selection : dependencyOrder) {
        auto id = selection.specifier.toString();
        if (m_fetcher->hasResult(id)) {
            auto result = m_fetcher->getResult(id);
            TU_RETURN_IF_NOT_OK (result.status);
            TU_RETURN_IF_STATUS (importPackageCache->installPackage(result.path));
        }
    }

    return targetBases;
}

bool
zuri_build::ImportSolver::packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const
{
    auto importPackageCache = m_packageManager->getIcache();
    auto environmentPackageCache = m_packageManager->getEcache();
    return importPackageCache->containsPackage(specifier) || environmentPackageCache->containsPackage(specifier);
}