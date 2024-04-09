add_executable(unit_tests
    main.c
    queue.c
    mem_pool.c
    map.c
    set.c
    cli.c
    file.c
    log.c
    mem_stream.c
    immutable_set.c
    union_find.c
    str_pool.c
    str.c
    graph.c
    heap.c)

if (TARGET overture_thread_pool)
    target_sources(unit_tests PRIVATE thread_pool.c)
    target_link_libraries(unit_tests PRIVATE overture_thread_pool)
endif()

target_include_directories(unit_tests PRIVATE ../src)
target_link_libraries(unit_tests PRIVATE
    overture_test
    overture_graph
    overture_mem_pool
    overture_str_pool
    overture_log)

add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME unit_tests_filter COMMAND unit_tests cli set str)
add_custom_target(memcheck
    COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS unit_tests)

if (ENABLE_COVERAGE)
    add_dependencies(coverage memcheck)
endif()
