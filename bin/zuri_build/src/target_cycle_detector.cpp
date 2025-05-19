
#include <boost/graph/depth_first_search.hpp>

#include <zuri_build/build_graph.h>
#include <zuri_build/target_cycle_detector.h>

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

    void discover_vertex(BuildGraphVertex v, const BuildGraphImpl &g) {
        auto target_name = get(vertex_target_name_t(), g);
        const std::string name = boost::get(target_name, v);
        m_dependencyPath.push_back(name);
    }

    void finish_vertex(BuildGraphVertex v, const BuildGraphImpl &g) {
        auto target_name = get(vertex_target_name_t(), g);
        const std::string name = boost::get(target_name, v);
        m_dependencyPath.pop_back();
    }

    void back_edge(BuildGraphEdge e, const BuildGraphImpl &g) {
        auto target_name = get(vertex_target_name_t(), g);
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

TargetCycleDetector::TargetCycleDetector(
    std::shared_ptr<BuildGraph> buildGraph,
    absl::flat_hash_set<std::vector<std::string>> targetCycles)
    : m_buildGraph(std::move(buildGraph)),
      m_targetCycles(std::move(targetCycles))
{
    TU_ASSERT (m_buildGraph != nullptr);
}

tempo_utils::Result<std::shared_ptr<TargetCycleDetector>>
TargetCycleDetector::create(std::shared_ptr<BuildGraph> buildGraph)
{
    if (buildGraph == nullptr)
        tempo_config::ConfigStatus::forCondition(
            tempo_config::ConfigCondition::kConfigInvariant, "invalid build graph");

    absl::flat_hash_set<std::vector<std::string>> targetCycles;
    std::vector<std::string> dependencyPath;
    TargetDfsVisitor vis(targetCycles, dependencyPath);
    boost::depth_first_search(*buildGraph->m_buildGraph, boost::visitor(vis));

    return std::shared_ptr<TargetCycleDetector>(new TargetCycleDetector(
        std::move(buildGraph),
        std::move(targetCycles)));
}

bool
TargetCycleDetector::hasCycles() const
{
    return !m_targetCycles.empty();
}

absl::flat_hash_set<std::vector<std::string>>::const_iterator
TargetCycleDetector::cyclesBegin() const
{
    return m_targetCycles.cbegin();
}

absl::flat_hash_set<std::vector<std::string>>::const_iterator
TargetCycleDetector::cyclesEnd() const
{
    return m_targetCycles.cend();
}

int
TargetCycleDetector::numCycles() const
{
    return m_targetCycles.size();
}
