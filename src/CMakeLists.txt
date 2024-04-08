add_library(overture
    overture/str_pool.c
    overture/log.c
    overture/mem_pool.c
    overture/graph.c)
target_include_directories(overture PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

find_package(Threads QUIET)
if (Threads_FOUND)
    add_library(overture_thread_pool overture/thread_pool.c)
    target_link_libraries(overture_thread_pool PUBLIC overture Threads::Threads)
    install(TARGETS overture_thread_pool EXPORT overture)
endif()

add_library(overture_test overture/test.c)
target_link_libraries(overture_test PUBLIC overture)
if (WIN32 OR TEST_DISABLE_FORK)
    target_compile_definitions(overture_test PRIVATE -DTEST_DISABLE_FORK)
else()
    message(STATUS "Building tests with process isolation enabled")
endif()

install(TARGETS overture overture_test EXPORT overture)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    TYPE INCLUDE
    FILES_MATCHING PATTERN "*.h")

install(
    EXPORT overture
    FILE overture-config.cmake
    NAMESPACE overture::
    DESTINATION lib/cmake/overture)
