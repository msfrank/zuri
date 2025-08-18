#ifndef ZURI_PKG_INSTALL_SOLVER_H
#define ZURI_PKG_INSTALL_SOLVER_H

#include <zuri_distributor/abstract_package_resolver.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_tooling/package_manager.h>

namespace zuri_pkg {

    class InstallSolver {
    public:
        explicit InstallSolver(
            std::shared_ptr<zuri_tooling::PackageManager> packageManager,
            bool systemInstall,
            bool dryRun);

        tempo_utils::Status configure();

        tempo_utils::Status addPackage(const zuri_packager::PackageId &id);
        tempo_utils::Status addPackage(const zuri_packager::PackageSpecifier &specifier);
        tempo_utils::Status addPackage(const tempo_utils::Url &url);

        tempo_utils::Status installPackages();

    private:
        std::shared_ptr<zuri_tooling::PackageManager> m_packageManager;
        bool m_systemInstall;
        bool m_dryRun;

        std::shared_ptr<zuri_distributor::PackageCache> m_dcache;
        std::shared_ptr<zuri_distributor::PackageCache> m_ucache;
        std::shared_ptr<zuri_distributor::PackageCache> m_installCache;
        std::shared_ptr<zuri_distributor::AbstractPackageResolver> m_resolver;
        std::unique_ptr<zuri_distributor::PackageFetcher> m_fetcher;
        std::unique_ptr<zuri_distributor::DependencySelector> m_selector;
        absl::flat_hash_set<tempo_utils::Url> m_packageUrls;

        bool packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const;
    };
}
#endif // ZURI_PKG_INSTALL_SOLVER_H