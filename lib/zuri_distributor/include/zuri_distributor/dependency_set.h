#ifndef ZURI_DISTRIBUTOR_DEPENDENCY_SET_H
#define ZURI_DISTRIBUTOR_DEPENDENCY_SET_H

#include <tempo_utils/result.h>
#include <tempo_utils/status.h>
#include <tempo_utils/url.h>
#include <zuri_packager/package_dependency.h>
#include <zuri_packager/package_types.h>

namespace zuri_distributor {

    class DependencySet {
    public:
        DependencySet();
        DependencySet(const DependencySet &other);

        tempo_utils::Status addDirectDependency(const zuri_packager::PackageSpecifier &dependency);
        tempo_utils::Result<bool> addTransitiveDependency(
            const zuri_packager::PackageSpecifier &target,
            const zuri_packager::PackageSpecifier &dependency);

        bool satisfiesDependency(const zuri_packager::PackageSpecifier &specifier) const;

        tempo_utils::Result<std::vector<zuri_packager::PackageSpecifier>> calculateResolutionOrder() const;

        struct Priv;

    private:
        std::shared_ptr<Priv> m_priv;
    };
}

#endif // ZURI_DISTRIBUTOR_DEPENDENCY_SET_H
