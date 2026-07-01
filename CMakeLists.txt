cmake_minimum_required(VERSION 3.21)
project(overture VERSION 0.0.2)

option(OVERTURE_TEST_DISABLE_FORK "Disables fork() in the testing framework." OFF)

if (PROJECT_IS_TOP_LEVEL)
    option(OVERTURE_ENABLE_COVERAGE          "Enables code coverage build type and target." OFF)
    option(OVERTURE_ENABLE_DOXYGEN           "Enables code documentation target via Doxygen." ON)
    option(OVERTURE_ENABLE_ADDRESS_SANITIZER "Enables the address sanitizer." OFF)
    option(OVERTURE_ENABLE_UNDEF_SANITIZER   "Enables the undefined value sanitizer." OFF)

    if (OVERTURE_ENABLE_ADDRESS_SANITIZER)
        add_compile_options($<$<C_COMPILER_ID:GNU,Clang>:-fsanitize=address>)
        add_link_options($<$<C_COMPILER_ID:GNU,Clang>:-fsanitize=address>)
    endif()

    if (OVERTURE_ENABLE_UNDEF_SANITIZER)
        add_compile_options($<$<C_COMPILER_ID:GNU,Clang>:-fsanitize=undefined>)
        add_link_options($<$<C_COMPILER_ID:GNU,Clang>:-fsanitize=undefined>)
    endif()

    if (OVERTURE_ENABLE_COVERAGE)
        include(cmake/Coverage.cmake)
    endif()

    if (OVERTURE_ENABLE_DOXYGEN)
        include(cmake/Doxygen.cmake)
    endif()
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
