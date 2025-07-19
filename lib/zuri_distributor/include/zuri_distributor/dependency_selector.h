#ifndef ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
#define ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H

#include "abstract_package_resolver.h"
#include "dependency_set.h"

namespace zuri_distributor {

    struct Selection {
        zuri_packager::PackageSpecifier specifier;
        tempo_utils::Url url;
        std::string shortcut;
    };

    class DependencySelector {
    public:
        explicit DependencySelector(std::shared_ptr<AbstractPackageResolver> resolver);

        tempo_utils::Status addDirectDependency(
            const zuri_packager::PackageSpecifier &dependency,
            std::string_view shortcut = {});
        tempo_utils::Status resolveTransitiveDependencies();

        tempo_utils::Result<std::vector<Selection>> calculateDependencyOrder();

    private:
        std::shared_ptr<AbstractPackageResolver> m_resolver;
        DependencySet m_dependencies;
        absl::flat_hash_map<zuri_packager::PackageSpecifier,PackageVersionDescriptor> m_descriptors;

        struct PendingSelection {
            zuri_packager::PackageSpecifier requested;
            zuri_packager::PackageSpecifier target;
        };
        std::queue<PendingSelection> m_pending;
    };
}

#endif // ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
