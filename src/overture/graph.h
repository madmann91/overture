#pragma once

#include "vec.h"
#include "set.h"
#include "map.h"
#include "span.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/// @file
///
/// Graph data structure, where edges and nodes are hashed by their label. Each node can carry a
/// number of additional user-defined pointers or integers that is defined during the construction
/// of the graph. This mechanism can be used to link the nodes of the graph to other data structures.

/// Iterates over the edges of a node, in the given direction.
/// @see graph_dir.
#define GRAPH_FOREACH_EDGE(edge, node, dir) \
    for (struct graph_edge* edge = graph_node_first_edge(node, dir); edge; \
        edge = graph_edge_next(edge, dir))

/// Iterates over the outgoing edges of a node.
/// @see GRAPH_FOREACH_EDGE.
#define GRAPH_FOREACH_OUTGOING_EDGE(edge, node) \
    GRAPH_FOREACH_EDGE(edge, node, GRAPH_DIR_FORWARD)

/// Iterates over the incoming edges of a node.
/// @see GRAPH_FOREACH_EDGE.
#define GRAPH_FOREACH_INCOMING_EDGE(edge, node) \
    GRAPH_FOREACH_EDGE(edge, node, GRAPH_DIR_BACKWARD)

struct graph_node;

/// Per-node or per-edge user data.
union graph_user_data {
    void* ptr;
    size_t index;
};

/// Graph edge.
struct graph_edge {
    struct graph_node* from;            /// Source node.
    struct graph_node* to;              /// Target node.
    struct graph_edge* next_in;         /// Next incoming edge (with same target), or `NULL`.
    struct graph_edge* next_out;        /// Next outgoing edge (with same source), or `NULL`.
    union graph_user_data user_data[];  /// Custom user data.
};

/// Reserved node indices.
enum graph_node_index {
    GRAPH_SOURCE_INDEX,
    GRAPH_SINK_INDEX,
    GRAPH_OTHER_INDEX
};

/// Graph node.
struct graph_node {
    size_t index;                       /// Unique index of the node.
    void* key;                          /// Key or label uniquely identifying the node.
    struct graph_edge* ins;             /// Incoming edges (edges which have this node as target), or `NULL`.
    struct graph_edge* outs;            /// Outgoing edges (edges which have this node as source), or `NULL`.
    union graph_user_data user_data[];  /// Custom user data.
};

SET_DECL(graph_edge_set, struct graph_edge*, PUBLIC)
VEC_DECL(graph_node_vec, struct graph_node*, PUBLIC)
SMALL_VEC_DECL(small_graph_node_vec, struct graph_node*, PUBLIC)
CONST_SPAN_DECL(const_graph_node_span, struct graph_node*)
SPAN_DECL(graph_node_span, struct graph_node*)
MAP_DECL(graph_node_map, struct graph_node*, void*, PUBLIC)
SET_DECL(graph_node_set, struct graph_node*, PUBLIC)

MAP_DECL(graph_node_key_map, void*, struct graph_node*, PUBLIC)

/// Graph data structure.
struct graph {
    size_t node_count;               /// Number of nodes in the graph.
    size_t edge_count;               /// Number of edges in the graph.
    size_t node_data_size;           /// Number of elements in the per-node data.
    size_t edge_data_size;           /// Number of elements in the per-edge data.
    struct graph_node* source;       /// Pointer to the source node of the graph.
    struct graph_node* sink;         /// Pointer to the sink node of the graph.
    struct graph_node_key_map nodes; /// Nodes of the graph.
    struct graph_edge_set edges;     /// Edges of the graph.
};

/// Graph traversal directions.
enum graph_dir {
    GRAPH_DIR_FORWARD,
    GRAPH_DIR_BACKWARD
};

/// Returns the opposite direction to the given one.
[[nodiscard]] enum graph_dir graph_dir_reverse(enum graph_dir);
/// Returns the first edge of the given node, in the given direction.
[[nodiscard]] struct graph_edge* graph_node_first_edge(const struct graph_node*, enum graph_dir);
/// Returns the edge following the given one, in the given direction.
[[nodiscard]] struct graph_edge* graph_edge_next(const struct graph_edge*, enum graph_dir);
/// Returns the target or source of the given edge, depending on the given direction.
[[nodiscard]] struct graph_node* graph_edge_endpoint(const struct graph_edge*, enum graph_dir);
/// Returns the source or the sink of the graph, depending on the given direction.
[[nodiscard]] struct graph_node* graph_source(struct graph*, enum graph_dir);
/// Returns the sink or the source of the graph, depending on the given direction.
[[nodiscard]] struct graph_node* graph_sink(struct graph*, enum graph_dir);

/// Creates a graph with a source and sink node.
/// @param node_data_size Number of elements in the `data` field for each node.
/// @param edge_data_size Number of elements in the `data` field for each edge.
/// @param source_key Key to use to identify the source node. May be `NULL`, in which case the
///   source node cannot be found via @ref graph_find.
/// @param sink_key Key to use to identify the sink node. May be `NULL`, in which case the
///   source node cannot be found via @ref graph_find.
[[nodiscard]] struct graph graph_create(
    size_t node_data_size,
    size_t edge_data_size,
    void* source_key,
    void* sink_key);

/// Destroys the given graph.
void graph_destroy(struct graph*);

/// Finds the node with the given key. Returns `NULL` if no such node exists.
[[nodiscard]] struct graph_node* graph_find(struct graph*, void* key);
/// Inserts a node with the given key.
struct graph_node* graph_insert(struct graph*, void* key);
/// Connects two nodes with an edge.
struct graph_edge* graph_connect(struct graph*, struct graph_node*, struct graph_node*);
/// Disconnects two nodes. The corresponding edge is destroyed.
/// @return `true` if the edge was removed, `false` otherwise (because the edge does not exist).
bool graph_disconnect(struct graph*, struct graph_node*, struct graph_node*);

/// Computes the post-order traversal of the graph, starting from the source or the sink of the
/// graph, depending on the traversal direction.
[[nodiscard]] struct graph_node_vec graph_compute_post_order(struct graph*, enum graph_dir);
/// Computes the depth-first-order traversal of the graph, starting from the source or the sink of
/// the graph, depending on the traversal direction.
[[nodiscard]] struct graph_node_vec graph_compute_depth_first_order(struct graph*, enum graph_dir);

/// Prints the graph on the given stream, in the `dot` format.
void graph_print(FILE*, const struct graph*);
/// Prints the graph on the standard output.
void graph_dump(const struct graph*);
