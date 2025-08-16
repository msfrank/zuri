#ifndef ZURI_TOOLING_INTERNAL_BUILD_GRAPH_IMPL_H
#define ZURI_TOOLING_INTERNAL_BUILD_GRAPH_IMPL_H

#include <boost/graph/adjacency_list.hpp>

namespace zuri_tooling::internal {

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

}

#endif // ZURI_TOOLING_INTERNAL_BUILD_GRAPH_IMPL_H