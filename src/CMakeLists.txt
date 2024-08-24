add_library(overture INTERFACE)
add_library(overture_str_pool overture/str_pool.c)
add_library(overture_mem_pool overture/mem_pool.c)
add_library(overture_log overture/log.c)
add_library(overture_graph overture/graph.c)
add_library(overture_test overture/test.c)
add_library(overture_file overture/file.c)

find_package(Threads QUIET)
if (Threads_FOUND)
    add_library(overture_thread_pool overture/thread_pool.c)
    target_link_libraries(overture_thread_pool PUBLIC overture Threads::Threads)
    install(TARGETS overture_thread_pool EXPORT overture)
endif()

target_include_directories(overture INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if (WIN32 OR TEST_DISABLE_FORK)
    target_compile_definitions(overture_test PRIVATE -DTEST_DISABLE_FORK)
else()
    message(STATUS "Building tests with process isolation enabled")
endif()

target_link_libraries(overture_test PUBLIC overture)
target_link_libraries(overture_str_pool PUBLIC overture)
target_link_libraries(overture_mem_pool PUBLIC overture)
target_link_libraries(overture_log PUBLIC overture)
target_link_libraries(overture_graph PUBLIC overture)
target_link_libraries(overture_file PUBLIC overture)

install(TARGETS
    overture
    overture_test
    overture_str_pool
    overture_mem_pool
    overture_log
    overture_graph
    overture_file
    EXPORT overture)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(
    EXPORT overture
    FILE overture-config.cmake
    NAMESPACE overture::
    DESTINATION lib/cmake/overture)
