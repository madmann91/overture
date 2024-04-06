if (ENABLE_DOXYGEN)
    find_package(Doxygen QUIET)
    if (DOXYGEN_FOUND)
        find_file(DOXYFILE Doxyfile HINTS doc/ PATHS ${PROJECT_SOURCE_DIR})
        if (DOXYFILE STREQUAL DOXYFILE-NOTFOUND)
            message(WARNING "Cannot find Doxyfile. Doxygen target cannot be created.")
        else()
            message(STATUS "Enabling Doxygen documentation target")
            add_custom_target(doxygen COMMAND Doxygen::doxygen ${DOXYFILE} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
        endif()
    else()
        message(WARNING "Cannot find Doxygen. Doxygen target cannot be created.")
    endif()
endif()
