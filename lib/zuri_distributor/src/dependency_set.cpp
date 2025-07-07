
#include <absl/container/flat_hash_map.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/icl/split_interval_map.hpp>

#include <zuri_distributor/dependency_set.h>
#include <zuri_distributor/distributor_result.h>

struct vertex_package_id_t {
    typedef boost::vertex_property_tag kind;
};

// collection of vertex properties
typedef boost::property<vertex_package_id_t, zuri_packager::PackageId
    > DependencyGraphVertexProperties;

// concrete graph type
typedef boost::adjacency_list<
    boost::vecS,                    // edge list type
    boost::vecS,                    // vertex list type
    boost::bidirectionalS,          // directed type
    DependencyGraphVertexProperties>
        DependencyGraphImpl;

typedef boost::graph_traits<DependencyGraphImpl>::vertex_descriptor DependencyGraphVertex;
typedef boost::graph_traits<DependencyGraphImpl>::edge_descriptor DependencyGraphEdge;
typedef boost::graph_traits<DependencyGraphImpl>::adjacency_iterator AdjacencyIterator;

struct PackageEntry {
    zuri_packager::PackageId id;
    boost::icl::split_interval_map<
        zuri_packager::PackageVersion,
        std::set<DependencyGraphVertex>
    > versions;
    DependencyGraphVertex vertex;
};

struct zuri_distributor::DependencySet::Priv {
    zuri_packager::PackageId rootId;
    DependencyGraphImpl graph;
    absl::flat_hash_map<zuri_packager::PackageId,PackageEntry> packages;
    DependencyGraphVertex rootVertex;
};

zuri_distributor::DependencySet::DependencySet()
{
}

zuri_distributor::DependencySet::DependencySet(const zuri_packager::PackageId &rootId)
    : m_priv(std::make_shared<Priv>())
{
    m_priv->rootId = rootId;

    m_priv->rootVertex = boost::add_vertex(m_priv->graph);
    auto package_id = get(vertex_package_id_t(), m_priv->graph);
    boost::put(package_id, m_priv->rootVertex, rootId);

    PackageEntry packageEntry;
    packageEntry.id = rootId;
    packageEntry.vertex = m_priv->rootVertex;

    m_priv->packages[rootId] = std::move(packageEntry);
}

zuri_distributor::DependencySet::DependencySet(const DependencySet &other)
    : m_priv(other.m_priv)
{
}

bool
zuri_distributor::DependencySet::isValid() const
{
    return m_priv != nullptr;
}

/**
 * Add dependency edge from the root package to the specified `dependency`.
 *
 * @param dependency
 * @return
 */
tempo_utils::Status
zuri_distributor::DependencySet::addDependency(const zuri_packager::PackageDependency &dependency)
{
    return addDependency(m_priv->rootId, dependency);
}

/**
 * Add dependency edge from the `targetId` package to the specified `dependency`.
 *
 * @param targetId
 * @param targetDependency
 * @return
 */
tempo_utils::Status
zuri_distributor::DependencySet::addDependency(
    const zuri_packager::PackageId &targetId,
    const zuri_packager::PackageDependency &targetDependency)
{
    if (!isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "invalid dependency set");

    // target package must exist in the graph
    auto targetIt = m_priv->packages.find(targetId);
    if (targetIt == m_priv->packages.cend())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing package target {}", targetId.toString());
    auto &targetEntry = targetIt->second;
    auto targetVertex = targetEntry.vertex;

    TU_LOG_VV << "target " << targetId.toString() << " has vertex " << (int) targetVertex;

    auto dependencyId = targetDependency.getPackageId();
    auto dependencyIt = m_priv->packages.find(dependencyId);

    // if dependency is not present in the graph then add it
    if (dependencyIt == m_priv->packages.cend()) {
        PackageEntry packageEntry;
        packageEntry.id = dependencyId;

        packageEntry.vertex = boost::add_vertex(m_priv->graph);
        auto package_id = get(vertex_package_id_t(), m_priv->graph);
        boost::put(package_id, packageEntry.vertex, dependencyId);

        TU_LOG_VV << "added vertex " << (int) packageEntry.vertex << " for dependency " << dependencyId.toString();

        auto ret = m_priv->packages.insert_or_assign(dependencyId, std::move(packageEntry));
        dependencyIt = ret.first;
    }
    auto &dependencyEntry = dependencyIt->second;

    // define an edge from the target package to the dependency
    auto ret = boost::add_edge(targetVertex, dependencyEntry.vertex, m_priv->graph);
    if (!ret.second)
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "dependency was already declared");

    TU_LOG_VV << "added edge from " << (int) targetVertex << " to " << (int) dependencyEntry.vertex;

    // add requirements to the package
    for (auto it = targetDependency.requirementsBegin(); it != targetDependency.requirementsEnd(); ++it) {
        const auto &req = *it;
        auto versionInterval = req->getInterval();
        auto interval = boost::icl::interval<zuri_packager::PackageVersion>::right_open(
            versionInterval.closedLowerBound, versionInterval.openUpperBound);
        std::set<DependencyGraphVertex> vertexSet{targetVertex};
        dependencyEntry.versions += std::make_pair(interval, vertexSet);
    }

    return {};
}

/**
 * Calculates the dependency resolution order by performing a topological sort of the packages
 * in the dependency set.
 *
 * @return
 */
tempo_utils::Result<std::vector<zuri_distributor::ResolvedPackage>>
zuri_distributor::DependencySet::calculateResolutionOrder() const
{
    if (!isValid())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "invalid dependency set");

    std::vector<ResolvedPackage> dependencyResolutionOrder;

    std::vector<DependencyGraphVertex> topologicalOrder;
    boost::topological_sort(m_priv->graph, std::back_insert_iterator(topologicalOrder),
        boost::vertex_index_map(boost::identity_property_map()));

    for (auto v : topologicalOrder) {
        auto numTargets = boost::in_degree(v, m_priv->graph);

        auto package_id = get(vertex_package_id_t(), m_priv->graph);
        const zuri_packager::PackageId id = boost::get(package_id, v);

        TU_LOG_VV << "found package " << id.toString() << " with " << (int) numTargets << " target in-edges";

        AdjacencyIterator it, end;
        for (boost::tie(it, end) = boost::adjacent_vertices(v, m_priv->graph); it != end; ++it) {
            auto tgt = *it;
            auto targetId = boost::get(package_id, tgt);
            TU_LOG_VV << "  depends on " << targetId.toString();
        }

        std::vector<zuri_packager::VersionInterval> validIntervals;
        const auto &packageEntry = m_priv->packages.at(id);
        for (const auto &entry : packageEntry.versions) {
            if (entry.second.size() < numTargets)
                continue;
            zuri_packager::VersionInterval interval(entry.first.lower(), entry.first.upper());
            validIntervals.push_back(std::move(interval));
        }

        ResolvedPackage resolved(id, std::move(validIntervals));
        dependencyResolutionOrder.push_back(std::move(resolved));
    }

    dependencyResolutionOrder.pop_back();

    return dependencyResolutionOrder;
}
