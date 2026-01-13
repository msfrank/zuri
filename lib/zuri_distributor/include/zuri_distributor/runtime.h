#ifndef ZURI_DISTRIBUTOR_RUNTIME_H
#define ZURI_DISTRIBUTOR_RUNTIME_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

#include "package_database.h"
#include "package_cache.h"
#include "package_cache_loader.h"
#include "tiered_package_cache.h"

namespace zuri_distributor {

    /**
     *
     */
    constexpr const char * const kPackagesDatabaseName = "packages.db";

    struct RuntimeOpenOrCreateOptions {
        bool exclusive = false;
        std::filesystem::path distributionLibDir = {};
        std::vector<std::filesystem::path> extraLibDirs = {};
        std::filesystem::path buildRoot = {};
    };

    /**
     *
     */
    class Runtime {
    public:
        ~Runtime();

        static tempo_utils::Result<std::shared_ptr<Runtime>> openOrCreate(
            const std::filesystem::path &runtimeRoot,
            const RuntimeOpenOrCreateOptions &options = {});
        static tempo_utils::Result<std::shared_ptr<Runtime>> open(const std::filesystem::path &runtimeRoot);

        std::filesystem::path getPackagesDatabaseFile() const;
        std::filesystem::path getBinDirectory() const;
        std::filesystem::path getLibDirectory() const;
        std::filesystem::path getPackagesDirectory() const;

        std::shared_ptr<lyric_runtime::AbstractLoader> getLoader() const;

        bool containsPackage(const zuri_packager::PackageSpecifier &specifier) const;
        tempo_utils::Result<Option<tempo_config::ConfigMap>> describePackage(
            const zuri_packager::PackageSpecifier &specifier) const;
        tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageSpecifier &specifier) const;

        tempo_utils::Result<std::filesystem::path> installPackage(const std::filesystem::path &packagePath);
        tempo_utils::Result<std::filesystem::path> installPackage(std::shared_ptr<zuri_packager::PackageReader> reader);
        tempo_utils::Status removePackage(const zuri_packager::PackageSpecifier &specifier);

    private:
        std::shared_ptr<PackageDatabase> m_packageDatabase;
        std::filesystem::path m_binDirectory;
        std::filesystem::path m_libDirectory;
        std::shared_ptr<PackageCache> m_packageStore;

        std::shared_ptr<PackageCacheLoader> m_loader;

        Runtime(
            std::shared_ptr<PackageDatabase> packageDatabase,
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            std::shared_ptr<PackageCache> packageStore);

        tempo_utils::Status configure();
    };

}

#endif // ZURI_DISTRIBUTOR_RUNTIME_H