cmake_minimum_required(VERSION 3.22)

project(tree_app
        VERSION 1.0.0
        DESCRIPTION "Provides interactive binary tree app."
        LANGUAGES CXX
)

# Setting language standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Adding executables
set(APP_SRC_FILES
    src/main.cpp
    src/application.cpp)

# Setting target name
set(TARGET_NAME tree_app)

# Creating target
add_executable(${TARGET_NAME})

# Setting private sources and include directory
target_sources(${TARGET_NAME} PRIVATE ${APP_SRC_FILES})

target_include_directories(${TARGET_NAME} PRIVATE include)

# Linking to tree_lib
target_link_libraries(${TARGET_NAME} tree_lib::tree_lib)

# Installing the executables
install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION bin
)
