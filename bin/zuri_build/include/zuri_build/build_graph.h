#ifndef ZURI_BUILD_BUILD_GRAPH_H
#define ZURI_BUILD_BUILD_GRAPH_H

#include <boost/graph/adjacency_list.hpp>

#include <lyric_build/lyric_builder.h>

#include "import_store.h"
#include "target_store.h"

struct vertex_target_name_t {
    typedef boost::vertex_property_tag kind;
};

// collection of vertex properties
typedef boost::property<vertex_target_name_t, std::string
    > BuildGraphVertexProperties;

// concrete graph type
typedef boost::adjacency_list<
    boost::vecS,                    // edge list type
    boost::vecS,                    // vertex list type
    boost::directedS,               // directed type
    BuildGraphVertexProperties>
        BuildGraphImpl;

typedef boost::graph_traits<BuildGraphImpl>::vertex_descriptor BuildGraphVertex;
typedef boost::graph_traits<BuildGraphImpl>::edge_descriptor BuildGraphEdge;

/**
 *
 */
class BuildGraph {
public:
    static tempo_utils::Result<std::shared_ptr<BuildGraph>> create(
        std::shared_ptr<TargetStore> targetStore,
        std::shared_ptr<ImportStore> importStore);

private:
    std::shared_ptr<TargetStore> m_targetStore;
    std::shared_ptr<ImportStore> m_importStore;

    std::unique_ptr<BuildGraphImpl> m_buildGraph;
    absl::flat_hash_map<std::string,BuildGraphVertex> m_targetsMap;
    absl::flat_hash_map<std::string,BuildGraphVertex> m_importsMap;
    absl::flat_hash_set<tempo_utils::Url> m_requestedPackages;
    absl::flat_hash_set<std::string> m_requestedRequirements;

    BuildGraph(
        std::shared_ptr<TargetStore> targetStore,
        std::shared_ptr<ImportStore> importStore,
        std::unique_ptr<BuildGraphImpl> buildGraph,
        absl::flat_hash_map<std::string,BuildGraphVertex> targetsMap,
        absl::flat_hash_map<std::string,BuildGraphVertex> importsMap,
        absl::flat_hash_set<tempo_utils::Url> requestedPackages,
        absl::flat_hash_set<std::string> requestedRequirements);

    friend class TargetCycleDetector;
};

#endif // ZURI_BUILD_BUILD_GRAPH_H
