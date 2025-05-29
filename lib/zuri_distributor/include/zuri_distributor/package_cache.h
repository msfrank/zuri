#ifndef ZURI_DISTRIBUTOR_PACKAGE_CACHE_H
#define ZURI_DISTRIBUTOR_PACKAGE_CACHE_H

#include <tempo_utils/result.h>
#include <zuri_distributor/distributor_result.h>
#include <zuri_packager/package_dependency.h>
#include <zuri_packager/package_reader.h>

namespace zuri_distributor {

    class PackageCache {
    public:
        static tempo_utils::Result<std::shared_ptr<PackageCache>> create(
            const std::filesystem::path &distributionRoot,
            std::string_view cacheName);
        static tempo_utils::Result<std::shared_ptr<PackageCache>> open(
            const std::filesystem::path &cacheDirectory);

        tempo_utils::Result<std::filesystem::path> installPackage(const std::filesystem::path &packagePath);
        tempo_utils::Result<std::filesystem::path> installPackage(std::shared_ptr<zuri_packager::PackageReader> reader);
        tempo_utils::Status removePackage(const zuri_packager::PackageSpecifier &specifier);

        tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageSpecifier &specifier);
        tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageDependency &dependency);

    private:
        std::filesystem::path m_cacheDirectory;

        explicit PackageCache(const std::filesystem::path &cacheDirectory);
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_CACHE_H
