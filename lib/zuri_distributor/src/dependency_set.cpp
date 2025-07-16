
#include <absl/container/flat_hash_map.h>
#include <absl/container/btree_set.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/icl/split_interval_map.hpp>

#include <zuri_distributor/dependency_set.h>
#include <zuri_distributor/distributor_result.h>

struct PackageMajor {
    zuri_packager::PackageId id;
    tu_uint32 major;

    bool operator==(const PackageMajor &other) const {
        return id == other.id && major == other.major;
    }
    bool operator<(const PackageMajor &other) const {
        auto idcmp = id.compare(other.id);
        if (idcmp != 0)
            return idcmp < 0;
        return major < other.major;
    }
    static PackageMajor fromSpecifier(const zuri_packager::PackageSpecifier &specifier) {
        PackageMajor packageMajor;
        packageMajor.id = specifier.getPackageId();
        packageMajor.major = specifier.getMajorVersion();
        return packageMajor;
    }

    template <typename H>
    friend H AbslHashValue(H h, const PackageMajor &major) {
        return H::combine(std::move(h), major.id, major.major);
    }
};

struct vertex_package_major_t {
    typedef boost::vertex_property_tag kind;
};

// collection of vertex properties
typedef boost::property<vertex_package_major_t, PackageMajor
    > VertexProperties;

// concrete graph type
typedef boost::adjacency_list<
    boost::vecS,                    // edge list type
    boost::vecS,                    // vertex list type
    boost::bidirectionalS,          // directed type
    VertexProperties>
        DependencyGraphImpl;

typedef boost::graph_traits<DependencyGraphImpl>::vertex_descriptor Vertex;
typedef boost::graph_traits<DependencyGraphImpl>::edge_descriptor Edge;
typedef boost::graph_traits<DependencyGraphImpl>::adjacency_iterator AdjacencyIterator;

struct MajorVersionEntry {
    PackageMajor major;
    zuri_packager::PackageVersion minimum;
    Vertex vertex;
};

struct zuri_distributor::DependencySet::Priv {
    //zuri_packager::PackageId rootId;
    Vertex rootVertex;
    absl::flat_hash_map<PackageMajor,std::unique_ptr<MajorVersionEntry>> majors;
    DependencyGraphImpl graph;
};

zuri_distributor::DependencySet::DependencySet()
    : m_priv(std::make_shared<Priv>())
{
    m_priv->rootVertex = boost::add_vertex(m_priv->graph);
}

zuri_distributor::DependencySet::DependencySet(const DependencySet &other)
    : m_priv(other.m_priv)
{
}

MajorVersionEntry *
get_or_add_major_version(
    const zuri_packager::PackageSpecifier &specifier,
    std::shared_ptr<zuri_distributor::DependencySet::Priv> priv)
{
    const auto major = PackageMajor::fromSpecifier(specifier);

    // check whether package major already exists in majors
    auto entry = priv->majors.find(major);

    if (entry != priv->majors.cend())
        return entry->second.get();

    // if package major does not exist then create a new entry and return pointer
    auto majorVersionEntry = std::make_unique<MajorVersionEntry>();
    majorVersionEntry->major = major;
    majorVersionEntry->vertex = boost::add_vertex(priv->graph);
    auto *mvPtr = majorVersionEntry.get();
    priv->majors[major] = std::move(majorVersionEntry);

    // tag vertex with specifier
    auto package_major = get(vertex_package_major_t(), priv->graph);
    boost::put(package_major, mvPtr->vertex, major);

    return mvPtr;
}

tempo_utils::Status
add_dependency_edge(
    Vertex targetVertex,
    MajorVersionEntry *mvPtr,
    const zuri_packager::PackageSpecifier &specifier,
    std::shared_ptr<zuri_distributor::DependencySet::Priv> &priv)
{
    auto dependencyVertex = mvPtr->vertex;

    // define an edge from the target vertex to the major version vertex
    auto ret = boost::add_edge(targetVertex, dependencyVertex, priv->graph);
    if (!ret.second)
        return zuri_distributor::DistributorStatus::forCondition(
            zuri_distributor::DistributorCondition::kDistributorInvariant,
            "dependency was already declared");

    auto packageVersion = specifier.getPackageVersion();
    if (mvPtr->minimum < packageVersion) {
        mvPtr->minimum = packageVersion;
    }

    return {};
}

/**
 * Add dependency edge from the root package to the specified `dependency`.
 *
 * @param dependency
 * @return
 */
tempo_utils::Status
zuri_distributor::DependencySet::addDirectDependency(const zuri_packager::PackageSpecifier &dependency)
{
    auto *mvPtr = get_or_add_major_version(dependency, m_priv);
    TU_RETURN_IF_NOT_OK (add_dependency_edge(m_priv->rootVertex, mvPtr, dependency, m_priv));
    return {};
}

/**
 * Add dependency edge from the `targetId` package to the specified `dependency`.
 *
 * @param targetId
 * @param targetDependency
 * @return
 */
tempo_utils::Result<bool>
zuri_distributor::DependencySet::addTransitiveDependency(
    const zuri_packager::PackageSpecifier &target,
    const zuri_packager::PackageSpecifier &dependency)
{
    TU_ASSERT (target.isValid());

    // target major version must exist in the graph
    const auto targetMajor = PackageMajor::fromSpecifier(target);
    auto entry = m_priv->majors.find(targetMajor);
    if (entry == m_priv->majors.cend())
        return DistributorStatus::forCondition(DistributorCondition::kDistributorInvariant,
            "missing package target {}", target.toString());
    auto *targetMvPtr = entry->second.get();

    // if dependency exists with a higher minimum version then return false
    auto *depMvPtr = get_or_add_major_version(dependency, m_priv);
    if (depMvPtr->minimum >= dependency.getPackageVersion())
        return false;

    // create edge from target mv to dependency mv
    TU_RETURN_IF_NOT_OK (add_dependency_edge(targetMvPtr->vertex, depMvPtr, dependency, m_priv));

    // return true to indicate a new minimum version was selected
    return true;
}

/**
 *
 * @param specifier
 * @return
 */
bool
zuri_distributor::DependencySet::satisfiesDependency(const zuri_packager::PackageSpecifier &specifier) const
{
    TU_ASSERT (specifier.isValid());
    const auto packageMajor = PackageMajor::fromSpecifier(specifier);
    auto entry = m_priv->majors.find(packageMajor);
    if (entry == m_priv->majors.cend())
        return false;
    const auto &mv = entry->second;
    return mv->minimum >= specifier.getPackageVersion();
}

/**
 * Calculates the dependency resolution order by performing a topological sort of the packages
 * in the dependency set.
 *
 * @return
 */
tempo_utils::Result<std::vector<zuri_packager::PackageSpecifier>>
zuri_distributor::DependencySet::calculateResolutionOrder() const
{
    std::vector<zuri_packager::PackageSpecifier> dependencyResolutionOrder;

    std::vector<Vertex> topologicalOrder;
    boost::topological_sort(m_priv->graph, std::back_insert_iterator(topologicalOrder),
        boost::vertex_index_map(boost::identity_property_map()));

    for (auto v : topologicalOrder) {
        if (v == m_priv->rootVertex)
            continue;

        auto package_major = get(vertex_package_major_t(), m_priv->graph);
        const auto major = boost::get(package_major, v);
        const auto &mv = m_priv->majors.at(major);
        zuri_packager::PackageSpecifier selection(major.id, mv->minimum);

        TU_LOG_INFO << "selected package " << selection.toString();

        AdjacencyIterator it, end;
        for (boost::tie(it, end) = boost::adjacent_vertices(v, m_priv->graph); it != end; ++it) {
            auto target = *it;
            auto targetMajor = boost::get(package_major, target);
            TU_LOG_INFO << "  depends on " << targetMajor.id.toString() << ":" << targetMajor.major;
        }

        dependencyResolutionOrder.push_back(std::move(selection));
    }

    return dependencyResolutionOrder;
}
