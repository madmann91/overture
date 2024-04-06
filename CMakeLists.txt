cmake_minimum_required(VERSION 3.16)
project(overture VERSION 0.0.1)

option(TEST_DISABLE_FORK "Disable fork() in the testing framework." OFF)
option(ENABLE_COVERAGE "Enable coverage target" OFF)
option(ENABLE_DOXYGEN "Enable doxygen target" ON)

add_compile_options(
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall>
    $<$<C_COMPILER_ID:GNU,Clang>:-Wextra>
    $<$<C_COMPILER_ID:GNU,Clang>:-pedantic>)

include(cmake/Coverage.cmake)
include(cmake/Doxygen.cmake)
install(
    FILES
        cmake/Coverage.cmake
        cmake/Doxygen.cmake
    DESTINATION libexec/overture/)

set(CMAKE_C_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)

include(CTest)
if (BUILD_TESTING)
    add_subdirectory(test)
endif()
