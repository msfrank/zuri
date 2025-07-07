#ifndef ZURI_DISTRIBUTOR_DEPENDENCY_SET_H
#define ZURI_DISTRIBUTOR_DEPENDENCY_SET_H

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>
#include <tempo_utils/url.h>
#include <zuri_packager/package_dependency.h>
#include <zuri_packager/package_types.h>

namespace zuri_distributor {

    struct ResolvedPackage {
        zuri_packager::PackageId packageId;
        std::vector<zuri_packager::VersionInterval> validIntervals;
    };

    class DependencySet {
    public:
        DependencySet();
        DependencySet(const zuri_packager::PackageId &rootId);
        DependencySet(const DependencySet &other);

        bool isValid() const;

        tempo_utils::Status addDependency(const zuri_packager::PackageDependency &dependency);
        tempo_utils::Status addDependency(
            const zuri_packager::PackageId &targetId,
            const zuri_packager::PackageDependency &targetDependency);

        tempo_utils::Result<std::vector<ResolvedPackage>> calculateResolutionOrder() const;

    private:
        struct Priv;
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_DISTRIBUTOR_DEPENDENCY_SET_H
