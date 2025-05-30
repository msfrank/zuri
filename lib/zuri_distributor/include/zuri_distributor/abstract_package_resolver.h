#ifndef ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H

#include <tempo_utils/option_template.h>
#include <tempo_utils/result.h>
#include <tempo_utils/url.h>

#include <zuri_packager/package_dependency.h>
#include <zuri_packager/package_specifier.h>

namespace zuri_distributor {

    struct PackageReference {
        zuri_packager::PackageSpecifier specifier;
        std::vector<tempo_utils::Url> referenceUrls;
        tu_int64 expiryTime;
    };

    class AbstractPackageResolver {
    public:
        virtual ~AbstractPackageResolver() = default;

        virtual tempo_utils::Result<Option<PackageReference>> resolveDependency(
            zuri_packager::PackageDependency &dependency) = 0;
    };
}

#endif // ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
