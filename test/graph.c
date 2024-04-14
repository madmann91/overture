#include <overture/test.h>
#include <overture/graph.h>
#include <overture/mem_stream.h>

TEST(graph) {
    int array[3] = {};
    struct graph graph = graph_create(1, 0, &array[0], &array[1]);
    graph_connect(&graph,
        graph_insert(&graph, &array[0]),
        graph_insert(&graph, &array[2]));
    graph_connect(&graph,
        graph_insert(&graph, &array[2]),
        graph_insert(&graph, &array[2]));
    graph_connect(&graph,
        graph_insert(&graph, &array[2]),
        graph_insert(&graph, &array[1]));
    struct graph_node* mid = graph_find(&graph, &array[2]);
    REQUIRE(mid);

    REQUIRE(graph_node_first_edge(graph.source, GRAPH_DIR_BACKWARD) == NULL);
    REQUIRE(graph_node_first_edge(graph.sink, GRAPH_DIR_FORWARD) == NULL);
    REQUIRE(graph_edge_next(graph.source->outs, GRAPH_DIR_FORWARD) == NULL);
    REQUIRE(graph_edge_endpoint(graph.source->outs, GRAPH_DIR_FORWARD) == mid);
    REQUIRE(graph_edge_next(graph.sink->ins, GRAPH_DIR_BACKWARD) == NULL);
    REQUIRE(graph_edge_endpoint(graph.sink->ins, GRAPH_DIR_BACKWARD) == mid);
    GRAPH_FOREACH_INCOMING_EDGE(in, mid) {
        REQUIRE(in->from == mid || in->from == graph_source(&graph, GRAPH_DIR_FORWARD));
    }
    GRAPH_FOREACH_OUTGOING_EDGE(out, mid) {
        REQUIRE(out->to == mid || out->to == graph_sink(&graph, GRAPH_DIR_FORWARD));
    }

    struct mem_stream mem_stream;
    mem_stream_init(&mem_stream);
    graph_print(mem_stream.file, &graph);
    mem_stream_destroy(&mem_stream);
    free(mem_stream.buf);

    struct graph_node_vec post_order = graph_compute_post_order(&graph, graph_dir_reverse(GRAPH_DIR_BACKWARD));
    REQUIRE(post_order.elem_count == 3);
    REQUIRE(post_order.elems[0] == graph.sink);
    REQUIRE(post_order.elems[1] == mid);
    REQUIRE(post_order.elems[2] == graph.source);
    graph_node_vec_destroy(&post_order);

    struct graph_node_vec depth_first_order = graph_compute_depth_first_order(&graph, GRAPH_DIR_FORWARD);
    REQUIRE(depth_first_order.elem_count == 3);
    REQUIRE(depth_first_order.elems[0] == graph.source);
    REQUIRE(depth_first_order.elems[1] == mid);
    REQUIRE(depth_first_order.elems[2] == graph.sink);
    graph_node_vec_destroy(&depth_first_order);

    graph_destroy(&graph);
}
