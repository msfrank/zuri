#ifndef ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
#define ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H

#include <tempo_utils/result.h>

#include <zuri_packager/package_dependency.h>

#include "dependency_set.h"

namespace zuri_distributor {

    class AbstractPackageResolver {
    public:
        virtual ~AbstractPackageResolver() = default;

        virtual tempo_utils::Result<DependencySet> resolveDependencySet(
            absl::flat_hash_map<zuri_packager::PackageId,zuri_packager::RequirementsList> &dependencies) = 0;
    };
}

#endif // ZURI_DISTRIBUTOR_ABSTRACT_PACKAGE_RESOLVER_H
