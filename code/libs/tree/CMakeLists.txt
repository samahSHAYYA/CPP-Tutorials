cmake_minimum_required(VERSION 3.22)

# Note that we always use underscored project lower-case names (instead of
# hyphened ones) to avoid potential conflicts (consider it as a best practice)
project(tree_lib
        VERSION 1.0.0
        DESCRIPTION "Provides binary tree data structures and functions."
        LANGUAGES CXX
)

# Setting language standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Adding global options regarding building documentation and test
if (NOT DEFINED BUILD_DOCS)
    option(BUILD_DOCS "Build documentation" ON)
endif()

if(NOT DEFINED BUILD_TESTS)
    option(BUILD_TESTS "Build tests" ON)
endif()

if(PROJECT_IS_TOP_LEVEL)
    set(DOCS_TARGET_NAME docs)
    set(CPACK_BASE_NAME "_")
else()
    set(DOCS_TARGET_NAME ${PROJECT_NAME}_docs)
    set(CPACK_BASE_NAME "_${PROJECT_NAME}_")
endif()

add_library(${PROJECT_NAME} INTERFACE)

# Adding a namespaced alias for the interface library
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
                            INTERFACE
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
                                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(${PROJECT_NAME}
                        PROPERTIES
                        OUTPUT_NAME ${PROJECT_NAME}
)

# Building documentation based on Doxygen
if(BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    if(NOT Doxygen_FOUND)
        message(FATAL_ERROR
                "Please install Doxygen to be able to build documentation or disable BUILD_DOCS.")
    endif()

    # Setting doxygen project name in upper case
    string(TOUPPER ${PROJECT_NAME} DOXY_PROJECT_NAME)

    set(DOXY_OUTPUT_REL_PATH docs)
    set(DOXY_GENERATE_MAN YES)     # NB: YES/NO are only allowed in Doxygen
    set(DOXY_MAN_REL_PATH  man)

    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})

    # Adding documentation target (you can generate docs by writing:
    # make docs if it root project else make tree_lib_docs if it is
    # a subproject)
    add_custom_target(${DOCS_TARGET_NAME}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM  # To execute command as it is (similar to raw expression)
    )

    # Adding clean documentation target (you can clean docs by writing:
    # make clean_docs if it is root project else write:
    # make clean_tree_lib_docs, which is the case of subproject)
    add_custom_target(clean_${DOCS_TARGET_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/${DOXY_OUTPUT_REL_PATH}
        COMMENT "Cleaning documentation"
    )
endif()

# Installing CMake package configuration files
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION include
)

# Installing headers
set(CMAKE_INSTALL_INCLUDEDIR include)
install(DIRECTORY include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Create package configuration file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion
)

configure_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in
               ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
)

# Install package configuration files
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

# Optional: Installing documentation
if (BUILD_DOCS AND DOXYGEN_FOUND)
    set(CMAKE_INSTALL_DOCDIR ${CMAKE_INSTALL_PREFIX}/${DOXY_OUTPUT_REL_PATH})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            DESTINATION ${CMAKE_INSTALL_DOCDIR})

    set(DOXYGEN_HTML_DIR
        ${CMAKE_CURRENT_BINARY_DIR}/${DOXY_OUTPUT_REL_PATH}/html)

    if(WIN32)
        install(DIRECTORY ${DOXYGEN_HTML_DIR}
                DESTINATION ${CMAKE_INSTALL_DOCDIR})
    else()
        install(DIRECTORY ${DOXYGEN_HTML_DIR}
                DESTINATION share/${DOXY_OUTPUT_REL_PATH}/${PROJECT_NAME})

        # Optionally, you can also install the generated man pages
        if(${DOXY_GENERATE_MAN})
            set(DOXYGEN_MAN_DIR
                ${CMAKE_CURRENT_BINARY_DIR}/${DOXY_OUTPUT_REL_PATH}/man)

            install(DIRECTORY ${DOXYGEN_MAN_DIR}
                    DESTINATION share)
        endif()
    endif()
endif()

# Testing
if (BUILD_TESTS)
    # Find GTest package
    find_package(GTest REQUIRED)

    if(NOT GTest_FOUND)
        message(FATAL_ERROR
                "Please install GTest to be able to build tests or disable BUILD_TESTS.")
    endif()

    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()

# Adding CPack configuration
set(CPACK${CPACK_BASE_NAME}PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK${CPACK_BASE_NAME}PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK${CPACK_BASE_NAME}PACKAGE_DESCRIPTION_SUMMARY "Binary Tree Library")
set(CPACK${CPACK_BASE_NAME}DEBIAN_PACKAGE_MAINTAINER "Samah A. SHAYYA")

# You can add vendor data
# set(CPACK${CPACK_BASE_NAME}PACKAGE_VENDOR "Add your vendor info")
# set(CPACK${CPACK_BASE_NAME}PACKAGE_CONTACT "Add your contact info")

# Set the package type based on the platform
if(WIN32)
    set(CPACK${CPACK_BASE_NAME}GENERATOR "ZIP")
elseif(UNIX)
    # MacOS or Linux
    set(CPACK${CPACK_BASE_NAME}GENERATOR "TGZ;DEB;RPM")

    # Used for RPM only to allow relocatable paths
    set(CPACK${CPACK_BASE_NAME}RPM_PACKAGE_RELOCATABLE ON)
    set(CPACK${CPACK_BASE_NAME}RPM_RELOCATION_PATHS "${CMAKE_INSTALL_PREFIX}")
endif()

# Including CPack after setting CPack configuration
include(CPack)
