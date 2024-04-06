#include "graph.h"
#include "hash.h"

#include <inttypes.h>
#include <assert.h>
#include <string.h>

static inline uint32_t hash_graph_edge(uint32_t h, struct graph_edge* const* edge_ptr) {
    h = hash_uint64(h, (*edge_ptr)->from->index);
    h = hash_uint64(h, (*edge_ptr)->to->index);
    return h;
}

static inline bool is_graph_edge_equal(
    struct graph_edge* const* edge_ptr,
    struct graph_edge* const* other_ptr)
{
    return
        (*edge_ptr)->from == (*other_ptr)->from &&
        (*edge_ptr)->to == (*other_ptr)->to;
}

static inline uint32_t hash_graph_node(uint32_t h, struct graph_node* const* node_ptr) {
    return hash_uint64(h, (*node_ptr)->index);
}

static inline bool is_graph_node_equal(
    struct graph_node* const* node_ptr,
    struct graph_node* const* other_ptr)
{
    return (*node_ptr)->key == (*other_ptr)->key;
}

static inline bool is_graph_node_ptr_equal(
    struct graph_node* const* node_ptr,
    struct graph_node* const* other_ptr)
{
    return *node_ptr == *other_ptr;
}

static inline uint32_t hash_raw_ptr(uint32_t h, void* const* ptr) {
    return hash_uint64(h, (uintptr_t)*ptr);
}

static inline bool is_raw_ptr_equal(void* const* ptr, void* const* other) {
    return *ptr == *other;
}

SET_IMPL(graph_node_set, struct graph_node*, hash_graph_node, is_graph_node_equal, PUBLIC)
SET_IMPL(graph_edge_set, struct graph_edge*, hash_graph_edge, is_graph_edge_equal, PUBLIC)
MAP_IMPL(graph_node_map, struct graph_node*, void*, hash_graph_node, is_graph_node_ptr_equal, PUBLIC)
VEC_IMPL(graph_node_vec, struct graph_node*, PUBLIC)
SMALL_VEC_IMPL(small_graph_node_vec, struct graph_node*, PUBLIC)

MAP_IMPL(graph_node_key_map, void*, struct graph_node*, hash_raw_ptr, is_raw_ptr_equal, PUBLIC)

enum graph_dir graph_dir_reverse(enum graph_dir dir) {
    return dir == GRAPH_DIR_FORWARD ? GRAPH_DIR_BACKWARD : GRAPH_DIR_FORWARD;
}

struct graph_edge* graph_node_first_edge(const struct graph_node* node, enum graph_dir dir) {
    return dir == GRAPH_DIR_FORWARD ? node->outs : node->ins;
}

struct graph_edge* graph_edge_next(const struct graph_edge* edge, enum graph_dir dir) {
    return dir == GRAPH_DIR_FORWARD ? edge->next_out : edge->next_in;
}

struct graph_node* graph_edge_endpoint(const struct graph_edge* edge, enum graph_dir dir) {
    return dir == GRAPH_DIR_FORWARD ? edge->to : edge->from;
}

struct graph_node* graph_source(struct graph* graph, enum graph_dir dir) {
    return dir == GRAPH_DIR_FORWARD ? graph->source : graph->sink;
}

struct graph_node* graph_sink(struct graph* graph, enum graph_dir dir) {
    return graph_source(graph, graph_dir_reverse(dir));
}

static inline struct graph_node* alloc_graph_node(size_t data_size) {
    return xcalloc(1, sizeof(struct graph_node) + data_size * sizeof(union graph_node_data));
}

struct graph graph_create(size_t data_size, void* source_key, void* sink_key) {
    assert((source_key == NULL && sink_key == NULL) || source_key != sink_key);

    struct graph_node* source = alloc_graph_node(data_size);
    struct graph_node* sink   = alloc_graph_node(data_size);
    source->index = GRAPH_SOURCE_INDEX;
    sink->index = GRAPH_SINK_INDEX;
    source->key = source_key;
    sink->key = sink_key;

    struct graph_node_key_map nodes = graph_node_key_map_create();
    if (source_key)
        graph_node_key_map_insert(&nodes, &source_key, &source);
    if (sink_key)
        graph_node_key_map_insert(&nodes, &sink_key, &sink);

    return (struct graph) {
        .source = source,
        .sink = sink,
        .node_count = GRAPH_OTHER_INDEX,
        .data_size = data_size,
        .nodes = nodes,
        .edges = graph_edge_set_create()
    };
}

void graph_destroy(struct graph* graph) {
    free(graph->source);
    free(graph->sink);

    MAP_FOREACH_VAL(struct graph_node*, node_ptr, graph->nodes) {
        if (*node_ptr != graph->source && *node_ptr != graph->sink)
            free(*node_ptr);
    }
    graph_node_key_map_destroy(&graph->nodes);

    SET_FOREACH(struct graph_edge*, edge_ptr, graph->edges) {
        free(*edge_ptr);
    }
    graph_edge_set_destroy(&graph->edges);
    memset(graph, 0, sizeof(struct graph));
}

struct graph_node* graph_find(struct graph* graph, void* key) {
    struct graph_node* const* node_ptr = graph_node_key_map_find(&graph->nodes, &key);
    return node_ptr ? *node_ptr : NULL;
}

struct graph_node* graph_insert(struct graph* graph, void* key) {
    struct graph_node* node = graph_find(graph, key);
    if (node)
        return node;

    node = alloc_graph_node(graph->data_size);
    node->index = graph->node_count++;
    node->key = key;
    [[maybe_unused]] bool was_inserted = graph_node_key_map_insert(&graph->nodes, &key, &node);
    assert(was_inserted);
    return node;
}

struct graph_edge* graph_connect(
    struct graph* graph,
    struct graph_node* from,
    struct graph_node* to)
{
    assert(from != graph->sink);
    assert(to != graph->source);

    struct graph_edge* edge = &(struct graph_edge) { .from = from, .to = to };
    struct graph_edge* const* edge_ptr = graph_edge_set_find(&graph->edges, &edge);
    if (edge_ptr)
        return *edge_ptr;

    edge = xmalloc(sizeof(struct graph_edge));
    edge->from = from;
    edge->to = to;
    edge->next_in = to->ins;
    edge->next_out = from->outs;
    to->ins = from->outs = edge;
    [[maybe_unused]] bool was_inserted = graph_edge_set_insert(&graph->edges, &edge);
    assert(was_inserted);
    return edge;
}

struct graph_node_vec graph_compute_post_order(struct graph* graph, enum graph_dir dir) {
    struct graph_node_vec post_order = graph_node_vec_create();
    struct graph_node_set visited_set = graph_node_set_create();
    struct graph_node_vec stack = graph_node_vec_create();
    struct graph_node* source = graph_source(graph, dir);
    graph_node_vec_push(&stack, &source);
    graph_node_set_insert(&visited_set, &source);
restart:
    while (stack.elem_count > 0) {
        struct graph_node* node = stack.elems[stack.elem_count - 1];
        GRAPH_FOREACH_EDGE(edge, node, dir) {
            struct graph_node* endpoint = graph_edge_endpoint(edge, dir);
            if (graph_node_set_insert(&visited_set, &endpoint)) {
                graph_node_vec_push(&stack, &endpoint);
                goto restart;
            }
        }
        graph_node_vec_push(&post_order, &node);
        graph_node_vec_pop(&stack);
    }
    graph_node_vec_destroy(&stack);
    graph_node_set_destroy(&visited_set);
    return post_order;
}

struct graph_node_vec graph_compute_depth_first_order(struct graph* graph, enum graph_dir dir) {
    struct graph_node_vec depth_first_order = graph_node_vec_create();
    struct graph_node_set visited_set = graph_node_set_create();
    struct graph_node_vec stack = graph_node_vec_create();
    struct graph_node* source = graph_source(graph, dir);
    graph_node_vec_push(&stack, &source);
    graph_node_set_insert(&visited_set, &source);
    graph_node_vec_push(&depth_first_order, &source);
restart:
    while (stack.elem_count > 0) {
        struct graph_node* node = stack.elems[stack.elem_count - 1];
        GRAPH_FOREACH_EDGE(edge, node, dir) {
            struct graph_node* endpoint = graph_edge_endpoint(edge, dir);
            if (graph_node_set_insert(&visited_set, &endpoint)) {
                graph_node_vec_push(&stack, &endpoint);
                graph_node_vec_push(&depth_first_order, &endpoint);
                goto restart;
            }
        }
        graph_node_vec_pop(&stack);
    }
    graph_node_vec_destroy(&stack);
    graph_node_set_destroy(&visited_set);
    return depth_first_order;
}

static inline void print_node(FILE* file, const struct graph_node* node) {
    if (node->index == GRAPH_SOURCE_INDEX)
        fprintf(file, "source");
    else if (node->index == GRAPH_SINK_INDEX)
        fprintf(file, "sink");
    else
        fprintf(file, "%"PRIu64, node->index);
}

void graph_print(FILE* file, const struct graph* graph) {
    fprintf(file, "digraph {\n");
    SET_FOREACH(const struct graph_edge*, edge_ptr, graph->edges) {
        const struct graph_edge* edge = *edge_ptr;
        fprintf(file, "    ");
        print_node(file, edge->from);
        fprintf(file, " -> ");
        print_node(file, edge->to);
        fprintf(file, "\n");
    }
    fprintf(file, "}\n");
}

void graph_dump(const struct graph* graph) {
    graph_print(stdout, graph);
    fflush(stdout);
}
