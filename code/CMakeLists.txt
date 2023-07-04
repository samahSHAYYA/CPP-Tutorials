cmake_minimum_required(VERSION 3.22)

project(CPP_Tutorials
        VERSION 1.0.0
        DESCRIPTION "It implements linked lists and binary trees."
        LANGUAGES CXX
)

# Option to build documentation (it is good to update documentation although
# it might take some considerable time)
option(BUILD_DOCS "Build documentation" ON)

# Option to build tests (it is good to build and run tests although it might
# take some considerable time)
option(BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Adding subdirectories (excluding tests as it is treated separately)
set(SUB_DIRS app libs)

foreach(SUB_DIR ${SUB_DIRS})
    add_subdirectory(${SUB_DIR})
endforeach()
