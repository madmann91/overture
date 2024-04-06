add_library(overture
    overture/str_pool.c
    overture/log.c
    overture/mem_pool.c
    overture/graph.c
    overture/test.c)
target_include_directories(overture PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

add_library(overture_test overture/test.c)
target_link_libraries(overture_test PUBLIC overture)
if (WIN32 OR TEST_DISABLE_FORK)
    target_compile_definitions(overture_test PUBLIC -DTEST_DISABLE_FORK)
else()
    message(STATUS "Building tests with process isolation enabled")
endif()

install(
    TARGETS overture overture_test
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
