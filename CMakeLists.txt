cmake_minimum_required(VERSION 3.21)
project(overture VERSION 0.0.1)

option(TEST_DISABLE_FORK "Disable fork() in the testing framework." OFF)

if (PROJECT_IS_TOP_LEVEL)
    option(ENABLE_COVERAGE "Enable code coverage build type and target." OFF)
    option(ENABLE_DOXYGEN "Enable code documentation target via Doxygen." ON)

    include(cmake/Coverage.cmake)
    include(cmake/Doxygen.cmake)
endif()

add_compile_options(
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall>
    $<$<C_COMPILER_ID:GNU,Clang>:-Wextra>
    $<$<C_COMPILER_ID:GNU,Clang>:-pedantic>)

install(
    FILES
        cmake/Coverage.cmake
        cmake/Doxygen.cmake
    DESTINATION libexec/overture/)

set(CMAKE_C_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

add_subdirectory(src)

if (PROJECT_IS_TOP_LEVEL)
    include(CTest)
    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
