if (ENABLE_COVERAGE)
    find_program(GCOVR_COMMAND gcovr REQUIRED)
    if (GCOVR_COMMAND)
        # Create a "coverage" target
        set(CMAKE_CXX_FLAGS_COVERAGE
            "-g -fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used by the C++ compiler during coverage builds."
            FORCE)
        set(CMAKE_C_FLAGS_COVERAGE
            "-g -fprofile-arcs -ftest-coverage"
            CACHE STRING "Flags used by the C++ compiler during coverage builds."
            FORCE)
        set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS_DEBUG}"
            CACHE STRING "Flags used for linking binaries during coverage builds."
            FORCE )
        set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}"
            CACHE STRING "Flags used by the shared libraries linker during coverage builds."
            FORCE )
        mark_as_advanced(
            CMAKE_CXX_FLAGS_COVERAGE
            CMAKE_C_FLAGS_COVERAGE
            CMAKE_EXE_LINKER_FLAGS_COVERAGE
            CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
        add_custom_target(coverage
            COMMENT "Generating coverage report"
            COMMAND ${GCOVR_COMMAND} -r ${PROJECT_SOURCE_DIR} --html coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    else()
        message(WARNING "Cannot find gcovr. Coverage target cannot be created.")
        add_custom_target(coverage)
    endif()
endif()
