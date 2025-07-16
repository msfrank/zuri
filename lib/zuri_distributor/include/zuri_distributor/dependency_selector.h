#ifndef ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
#define ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H

#include "abstract_package_resolver.h"
#include "dependency_set.h"

namespace zuri_distributor {

    struct PendingSelection {
        zuri_packager::PackageSpecifier requested;
        zuri_packager::PackageSpecifier target;
    };

    class DependencySelector {
    public:
        explicit DependencySelector(std::shared_ptr<AbstractPackageResolver> resolver);

        tempo_utils::Status addDirectDependency(const zuri_packager::PackageSpecifier &dependency);
        tempo_utils::Status resolveTransitiveDependencies();

        tempo_utils::Result<std::vector<zuri_packager::PackageSpecifier>> calculateDependencyOrder();

    private:
        std::shared_ptr<AbstractPackageResolver> m_resolver;
        DependencySet m_dependencies;
        std::queue<PendingSelection> m_pending;
    };
}

#endif // ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
