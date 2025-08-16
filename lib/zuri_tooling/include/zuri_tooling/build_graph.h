#ifndef ZURI_TOOLING_BUILD_GRAPH_H
#define ZURI_TOOLING_BUILD_GRAPH_H

#include <lyric_build/lyric_builder.h>
#include <zuri_tooling/import_store.h>
#include <zuri_tooling/target_store.h>

namespace zuri_tooling {

    class BuildOrder {
    public:
        BuildOrder();
        explicit BuildOrder(std::vector<std::string> &order);
        BuildOrder(const BuildOrder &other);

        bool isValid() const;

        std::string getRequestedTarget() const;
        std::string getTransitiveTarget(int index) const;
        int numTransitiveTargets() const;

        std::vector<std::string>::const_iterator orderBegin() const;
        std::vector<std::string>::const_iterator orderEnd() const;

    private:
        std::vector<std::string> m_order;
    };

    class BuildGraph {
    public:
        static tempo_utils::Result<std::shared_ptr<BuildGraph>> create(
            std::shared_ptr<TargetStore> targetStore,
            std::shared_ptr<ImportStore> importStore);

        std::shared_ptr<TargetStore> getTargetStore() const;
        std::shared_ptr<ImportStore> getImportStore() const;

        tempo_utils::Result<std::vector<std::string>> calculateBuildOrder(const std::string &targetName) const;

        bool hasCycles() const;
        absl::flat_hash_set<std::vector<std::string>>::const_iterator cyclesBegin() const;
        absl::flat_hash_set<std::vector<std::string>>::const_iterator cyclesEnd() const;
        int numCycles() const;

    private:
        std::shared_ptr<TargetStore> m_targetStore;
        std::shared_ptr<ImportStore> m_importStore;

        absl::flat_hash_set<tempo_utils::Url> m_requestedPackages;
        absl::flat_hash_set<zuri_packager::PackageSpecifier> m_requestedRequirements;
        absl::flat_hash_set<std::vector<std::string>> m_targetCycles;

        struct Priv;
        std::unique_ptr<Priv> m_priv;

        BuildGraph(
            std::shared_ptr<TargetStore> targetStore,
            std::shared_ptr<ImportStore> importStore,
            absl::flat_hash_set<tempo_utils::Url> &&requestedPackages,
            absl::flat_hash_set<zuri_packager::PackageSpecifier> &&requestedRequirements,
            absl::flat_hash_set<std::vector<std::string>> &&targetCycles,
            std::unique_ptr<Priv> &&priv);

        friend class TargetCycleDetector;
    };
}

#endif // ZURI_TOOLING_BUILD_GRAPH_H
