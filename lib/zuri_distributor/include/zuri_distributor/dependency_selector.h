#ifndef ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
#define ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H

#include <queue>

#include "abstract_package_resolver.h"
#include "dependency_set.h"

namespace zuri_distributor {

    struct Selection {
        std::string id;
        zuri_packager::PackageSpecifier specifier;
        tempo_utils::Url url;
        std::string shortcut;
    };

    class DependencySelector {
    public:
        explicit DependencySelector(std::shared_ptr<AbstractPackageResolver> resolver);

        tempo_utils::Result<std::string> addDirectDependency(const zuri_packager::PackageId &packageId,
            std::string_view shortcut = {});
        tempo_utils::Result<std::string> addDirectDependency(const zuri_packager::PackageSpecifier &packageSpecifier,
            std::string_view shortcut = {});
        tempo_utils::Result<std::string> addDirectDependency(const std::filesystem::path &packagePath,
            std::string_view shortcut = {});

        tempo_utils::Status selectDependencies();

        tempo_utils::Result<std::vector<Selection>> calculateDependencyOrder();

    private:
        std::shared_ptr<AbstractPackageResolver> m_resolver;
        DependencySet m_dependencies;
        absl::flat_hash_map<
            zuri_packager::PackageSpecifier,
            std::pair<std::string,tempo_utils::Url>> m_packageSelections;

        struct PendingSelection {
            enum class Type {
                Id,
                Specifier,
                Path,
                Transitive,
            };
            Type type;
            std::string id;
            zuri_packager::PackageId requestedId;
            zuri_packager::PackageSpecifier requestedSpecifier;
            std::filesystem::path requestedPath;
            zuri_packager::PackageSpecifier target;
            std::string shortcut;
        };
        std::queue<PendingSelection> m_pending;

        tempo_utils::Status dependOnLatestVersion(
            const std::string &id,
            const zuri_packager::PackageId &packageId,
            const std::string &shortcut);
        tempo_utils::Status dependOnSpecifiedVersion(
            const std::string &id,
            const zuri_packager::PackageSpecifier &packageSpecifier,
            const std::string &shortcut);
        tempo_utils::Status dependOnSpecifiedPath(
            const std::string &id,
            const std::filesystem::path &packagePath,
            const std::string &shortcut);
        tempo_utils::Status dependTransitively(
            const std::string &id,
            const zuri_packager::PackageSpecifier &target,
            const zuri_packager::PackageSpecifier &dependency);
    };
}

#endif // ZURI_DISTRIBUTOR_DEPENDENCY_SELECTOR_H
