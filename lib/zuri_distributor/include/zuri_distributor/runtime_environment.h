#ifndef ZURI_DISTRIBUTOR_RUNTIME_ENVIRONMENT_H
#define ZURI_DISTRIBUTOR_RUNTIME_ENVIRONMENT_H

#include <filesystem>

#include <lyric_runtime/abstract_loader.h>

#include "environment_database.h"
#include "package_cache.h"
#include "package_cache_loader.h"
#include "tiered_package_cache.h"

namespace zuri_distributor {

    /**
     *
     */
    constexpr const char * const kEnvironmentDatabaseName = "environment.db";

    struct RuntimeEnvironmentOptions {
        std::filesystem::path buildRoot = {};
    };

    /**
     *
     */
    class RuntimeEnvironment {
    public:
        ~RuntimeEnvironment();

        static tempo_utils::Result<std::shared_ptr<RuntimeEnvironment>> openOrCreate(
            const std::filesystem::path &environmentDirectory,
            const RuntimeEnvironmentOptions &options = {});
        static tempo_utils::Result<std::shared_ptr<RuntimeEnvironment>> open(
            const std::filesystem::path &environmentDirectoryOrDatabaseFile,
            const RuntimeEnvironmentOptions &options = {});

        std::filesystem::path getEnvironmentDatabaseFile() const;
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
        std::shared_ptr<EnvironmentDatabase> m_environmentDatabase;
        std::filesystem::path m_binDirectory;
        std::filesystem::path m_libDirectory;
        std::shared_ptr<PackageCache> m_packageStore;
        RuntimeEnvironmentOptions m_options;

        std::shared_ptr<PackageCacheLoader> m_loader;

        RuntimeEnvironment(
            std::shared_ptr<EnvironmentDatabase> environmentDatabase,
            const std::filesystem::path &binDirectory,
            const std::filesystem::path &libDirectory,
            std::shared_ptr<PackageCache> packageStore,
            const RuntimeEnvironmentOptions &options);

        tempo_utils::Status configure();
    };

}

#endif // ZURI_DISTRIBUTOR_RUNTIME_ENVIRONMENT_H