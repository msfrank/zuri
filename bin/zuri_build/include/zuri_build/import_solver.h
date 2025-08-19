#ifndef ZURI_BUILD_IMPORT_SOLVER_H
#define ZURI_BUILD_IMPORT_SOLVER_H

#include <lyric_importer/shortcut_resolver.h>
#include <zuri_distributor/abstract_package_resolver.h>
#include <zuri_distributor/dependency_selector.h>
#include <zuri_distributor/package_cache.h>
#include <zuri_distributor/package_fetcher.h>
#include <zuri_tooling/package_manager.h>

namespace zuri_build {

    class ImportSolver {
    public:
        explicit ImportSolver(std::shared_ptr<zuri_tooling::PackageManager> packageManager);

        tempo_utils::Status configure();

        tempo_utils::Status addImport(
            const zuri_packager::PackageSpecifier &specifier,
            std::string_view shortcut);
        tempo_utils::Status addImport(
            const tempo_utils::Url &url,
            std::string_view shortcut);

        tempo_utils::Status installImports(std::shared_ptr<lyric_importer::ShortcutResolver> shortcutResolver);

    private:
        std::shared_ptr<zuri_tooling::PackageManager> m_packageManager;
        std::shared_ptr<zuri_distributor::PackageCache> m_dcache;
        std::shared_ptr<zuri_distributor::PackageCache> m_ucache;
        std::shared_ptr<zuri_distributor::PackageCache> m_icache;
        std::shared_ptr<zuri_distributor::AbstractPackageResolver> m_resolver;
        std::unique_ptr<zuri_distributor::PackageFetcher> m_fetcher;
        std::unique_ptr<zuri_distributor::DependencySelector> m_selector;
        absl::flat_hash_map<tempo_utils::Url,std::string> m_urlShortcuts;

        bool packageIsPresent(const zuri_packager::PackageSpecifier &specifier) const;
    };
}

#endif // ZURI_BUILD_IMPORT_SOLVER_H
