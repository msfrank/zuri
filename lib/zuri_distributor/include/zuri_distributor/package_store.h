#ifndef ZURI_DISTRIBUTOR_PACKAGE_STORE_H
#define ZURI_DISTRIBUTOR_PACKAGE_STORE_H

#include <tempo_utils/result.h>
#include <zuri_packager/package_reader.h>

#include "abstract_package_cache.h"

namespace zuri_distributor {

    class PackageStore : public AbstractPackageCache {
    public:
        static tempo_utils::Result<std::shared_ptr<PackageStore>> openOrCreate(
            const std::filesystem::path &packagesDirectory);
        static tempo_utils::Result<std::shared_ptr<PackageStore>> open(
            const std::filesystem::path &packagesDirectory);

        std::filesystem::path getPackagesDirectory() const;

        bool containsPackage(const zuri_packager::PackageSpecifier &specifier) const override;
        tempo_utils::Result<Option<tempo_config::ConfigMap>> describePackage(
            const zuri_packager::PackageSpecifier &specifier) const override;
        tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageSpecifier &specifier) const override;

        tempo_utils::Result<std::filesystem::path> installPackage(std::shared_ptr<zuri_packager::PackageReader> reader);
        tempo_utils::Status removePackage(const zuri_packager::PackageSpecifier &specifier);


    private:
        std::filesystem::path m_packagesDirectory;

        explicit PackageStore(const std::filesystem::path &packagesDirectory);
    };
}

#endif // ZURI_DISTRIBUTOR_PACKAGE_STORE_H
