#ifndef ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H

#include <tempo_utils/result.h>

#include <zuri_packager/package_dependency.h>

#include "dependency_set.h"

namespace zuri_distributor {

    struct PackageVersionDescriptor {
        zuri_packager::PackageId id;
        zuri_packager::PackageVersion version;
        absl::flat_hash_set<zuri_packager::PackageSpecifier> dependencies;
        tempo_utils::Url url;
    };

    class AbstractPackageResolver {
    public:
        virtual ~AbstractPackageResolver() = default;

        virtual tempo_utils::Result<PackageVersionDescriptor> describePackageVersion(
            const zuri_packager::PackageId &packageId,
            const zuri_packager::PackageVersion &packageVersion) = 0;
    };
}

#endif // ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
