
#include <absl/strings/str_join.h>
#include <boost/graph/depth_first_search.hpp>

#include <zuri_tooling/build_graph.h>
#include <zuri_tooling/internal/build_graph_impl.h>

struct zuri_tooling::BuildGraph::Priv {
    internal::BuildGraphImpl buildGraph;
    absl::flat_hash_map<std::string,internal::BuildGraphVertex> targetsMap;
    absl::flat_hash_map<std::string,internal::BuildGraphVertex> importsMap;
};

namespace zuri_tooling {

    class BuildOrderingVisitor : public boost::default_dfs_visitor {
    public:
        explicit BuildOrderingVisitor(std::vector<std::string> &targetBuildOrder)
            : m_targetBuildOrder(targetBuildOrder)
        {
        }

        BuildOrderingVisitor(const BuildOrderingVisitor &other)
            : m_targetBuildOrder(other.m_targetBuildOrder)
        {
        }
        void initialize_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {};
        void start_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {};
        void discover_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {};
        void examine_edge(internal::BuildGraphEdge e, const internal::BuildGraphImpl &g) {};
        void tree_edge(internal::BuildGraphEdge e, const internal::BuildGraphImpl &g) {};
        void back_edge(internal::BuildGraphEdge e, const internal::BuildGraphImpl &g) {};
        void forward_or_cross_edge(internal::BuildGraphEdge e, const internal::BuildGraphImpl &g) {};

        void finish_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {
            auto target_name = get(internal::vertex_target_name_t(), g);
            const std::string name = boost::get(target_name, v);
            m_targetBuildOrder.push_back(name);
        }

    private:
        std::vector<std::string> &m_targetBuildOrder;
    };

    class TargetDfsVisitor : public boost::default_dfs_visitor {
    public:
        TargetDfsVisitor(
            absl::flat_hash_set<std::vector<std::string>> &targetCycles,
            std::vector<std::string> &dependencyPath)
            : m_targetCycles(targetCycles),
              m_dependencyPath(dependencyPath)
        {
        }

        TargetDfsVisitor(const TargetDfsVisitor &other)
            : m_targetCycles(other.m_targetCycles),
              m_dependencyPath(other.m_dependencyPath)
        {
        }

        void discover_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {
            auto target_name = get(internal::vertex_target_name_t(), g);
            const std::string name = boost::get(target_name, v);
            m_dependencyPath.push_back(name);
        }

        void finish_vertex(internal::BuildGraphVertex v, const internal::BuildGraphImpl &g) {
            auto target_name = get(internal::vertex_target_name_t(), g);
            const std::string name = boost::get(target_name, v);
            m_dependencyPath.pop_back();
        }

        void back_edge(internal::BuildGraphEdge e, const internal::BuildGraphImpl &g) {
            auto target_name = get(internal::vertex_target_name_t(), g);
            auto tgt = boost::target(e, g);
            const std::string tgtName = boost::get(target_name, tgt);
            auto cyclePath = m_dependencyPath;
            cyclePath.push_back(tgtName);
            m_targetCycles.insert(std::move(cyclePath));
        }

    private:
        absl::flat_hash_set<std::vector<std::string>> &m_targetCycles;
        std::vector<std::string> &m_dependencyPath;
    };
}

zuri_tooling::BuildGraph::BuildGraph(
    std::shared_ptr<TargetStore> targetStore,
    std::shared_ptr<ImportStore> importStore,
    absl::flat_hash_set<tempo_utils::Url> &&requestedPackages,
    absl::flat_hash_set<zuri_packager::PackageSpecifier> &&requestedRequirements,
    absl::flat_hash_set<std::vector<std::string>> &&targetCycles,
    std::unique_ptr<Priv> &&priv)
    : m_targetStore(std::move(targetStore)),
      m_importStore(std::move(importStore)),
      m_requestedPackages(std::move(requestedPackages)),
      m_requestedRequirements(std::move(requestedRequirements)),
      m_priv(std::move(priv))
{
    TU_ASSERT (m_targetStore != nullptr);
    TU_ASSERT (m_importStore != nullptr);
    TU_ASSERT (m_priv != nullptr);
}

tempo_utils::Result<std::shared_ptr<zuri_tooling::BuildGraph>>
zuri_tooling::BuildGraph::create(
    std::shared_ptr<TargetStore> targetStore,
    std::shared_ptr<ImportStore> importStore)
{
    auto priv = std::make_unique<Priv>();

    auto target_name = get(internal::vertex_target_name_t(), priv->buildGraph);

    absl::flat_hash_set<tempo_utils::Url> requestedPackages;
    absl::flat_hash_set<zuri_packager::PackageSpecifier> requestedRequirements;

    // construct map of target name to dependency index
    for (auto it = targetStore->targetsBegin(); it != targetStore->targetsEnd(); it++) {
        auto v = boost::add_vertex(priv->buildGraph);
        boost::put(target_name, v, it->first);
        priv->targetsMap[it->first] = v;
    }

    // construct map of import name to dependency index (if import refers to a target) and additionally
    // build the sets of requested packages and package requirements
    for (auto it = importStore->importsBegin(); it != importStore->importsEnd(); it++) {
        const auto &importName = it->first;
        const auto &importEntry = it->second;
        switch (importEntry->type) {
            case ImportEntryType::Target: {
                auto entry = priv->targetsMap.find(importEntry->targetName);
                if (entry == priv->targetsMap.cend())
                    return tempo_config::ConfigStatus::forCondition(
                        tempo_config::ConfigCondition::kConfigInvariant,
                        "import '{}' refers to nonexistent target '{}'", importName, importEntry->targetName);
                priv->importsMap[importName] = entry->second;
                break;
            }
            case ImportEntryType::Requirement: {
                requestedRequirements.insert(importEntry->requirementSpecifier);
                break;
            }
            case ImportEntryType::Package: {
                requestedPackages.insert(importEntry->packageUrl);
                break;
            }
            default:
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "invalid type for import '{}'", importName);
        }
    }

    // build list of dependency edges
    for (auto it = targetStore->targetsBegin(); it != targetStore->targetsEnd(); it++) {
        const auto &targetName = it->first;
        const auto &targetEntry = it->second;
        auto targetVertex = priv->targetsMap.at(it->first);

        for (const auto &targetDep : targetEntry->depends) {
            auto entry = priv->targetsMap.find(targetDep);
            if (entry == priv->targetsMap.cend())
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "target '{}' refers to nonexistent target dependency '{}'", targetName, targetDep);
            boost::add_edge(targetVertex, entry->second, priv->buildGraph);
        }

        for (const auto &targetImport : targetEntry->imports) {
            if (!importStore->hasImport(targetImport))
                return tempo_config::ConfigStatus::forCondition(
                    tempo_config::ConfigCondition::kConfigInvariant,
                    "target '{}' refers to nonexistent import '{}'", targetName, targetImport);
            const auto &importEntry = importStore->getImport(targetImport);
            switch (importEntry->type) {
                case ImportEntryType::Target: {
                    boost::add_edge(targetVertex, priv->importsMap.at(targetImport), priv->buildGraph);
                    break;
                }
                default:
                    break;
            }
        }
    }

    absl::flat_hash_set<std::vector<std::string>> targetCycles;
    std::vector<std::string> dependencyPath;
    TargetDfsVisitor vis(targetCycles, dependencyPath);
    boost::depth_first_search(priv->buildGraph, boost::visitor(vis));

    return std::shared_ptr<BuildGraph>(new BuildGraph(
        std::move(targetStore),
        std::move(importStore),
        std::move(requestedPackages),
        std::move(requestedRequirements),
        std::move(targetCycles),
        std::move(priv)));
}

std::shared_ptr<zuri_tooling::TargetStore>
zuri_tooling::BuildGraph::getTargetStore() const
{
    return m_targetStore;
}

std::shared_ptr<zuri_tooling::ImportStore>
zuri_tooling::BuildGraph::getImportStore() const
{
    return m_importStore;
}

tempo_utils::Result<std::vector<std::string>>
zuri_tooling::BuildGraph::calculateBuildOrder(const std::string &targetName) const
{
    auto entry = m_priv->targetsMap.find(targetName);
    if (entry == m_priv->targetsMap.cend())
        return tempo_config::ConfigStatus::forCondition(tempo_config::ConfigCondition::kMissingValue,
            "");

    std::vector<std::string> targetBuildOrder;
    BuildOrderingVisitor vis(targetBuildOrder);

    std::vector<boost::default_color_type> colors(boost::num_vertices(m_priv->buildGraph));
    auto vertex_index = boost::get(boost::vertex_index, m_priv->buildGraph);
    auto colorMap = boost::make_iterator_property_map(colors.begin(), vertex_index, colors[0]);

    boost::depth_first_visit(m_priv->buildGraph, entry->second, vis, colorMap);

    return targetBuildOrder;
}

bool
zuri_tooling::BuildGraph::hasCycles() const
{
    return !m_targetCycles.empty();
}

absl::flat_hash_set<std::vector<std::string>>::const_iterator
zuri_tooling::BuildGraph::cyclesBegin() const
{
    return m_targetCycles.cbegin();
}

absl::flat_hash_set<std::vector<std::string>>::const_iterator
zuri_tooling::BuildGraph::cyclesEnd() const
{
    return m_targetCycles.cend();
}

int
zuri_tooling::BuildGraph::numCycles() const
{
    return m_targetCycles.size();
}
