#ifndef ZURI_DISTRIBUTOR_ABSTRACT_READONLY_PACKAGE_CACHE_H
#define ZURI_DISTRIBUTOR_ABSTRACT_READONLY_PACKAGE_CACHE_H

#include <tempo_utils/result.h>
#include <zuri_packager/package_reader.h>

namespace zuri_distributor {

    class AbstractReadonlyPackageCache {
    public:
        virtual ~AbstractReadonlyPackageCache() = default;

        /**
         *
         * @param specifier
         * @return
         */
        virtual bool containsPackage(const zuri_packager::PackageSpecifier &specifier) const = 0;

        /**
         *
         * @param specifier
         * @return
         */
        virtual tempo_utils::Result<Option<tempo_config::ConfigMap>> describePackage(
            const zuri_packager::PackageSpecifier &specifier) const = 0;

        /**
         *
         * @param specifier
         * @return
         */
        virtual tempo_utils::Result<Option<std::filesystem::path>> resolvePackage(
            const zuri_packager::PackageSpecifier &specifier) const = 0;
    };
}

#endif // ZURI_DISTRIBUTOR_ABSTRACT_READONLY_PACKAGE_CACHE_H