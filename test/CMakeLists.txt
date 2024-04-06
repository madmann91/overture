add_executable(unit_tests
    main.c
    queue.c
    mem_pool.c
    map.c
    set.c
    cli.c
    io.c
    log.c
    mem_stream.c
    immutable_set.c
    union_find.c
    str_pool.c
    str.c
    graph.c
    heap.c)

target_include_directories(unit_tests PRIVATE ../src)
target_link_libraries(unit_tests PRIVATE overture_test)

add_test(NAME unit_tests COMMAND unit_tests)
add_test(NAME unit_tests_filter COMMAND unit_tests cli set str)
add_custom_target(memcheck
    COMMAND ${CMAKE_CTEST_COMMAND} -T memcheck
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS unit_tests)

if (ENABLE_COVERAGE)
    add_dependencies(coverage memcheck)
endif()
