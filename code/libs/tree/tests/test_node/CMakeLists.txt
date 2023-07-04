# Testing node.hpp
set(TEST_NAME test_node)

include_directories(${GTEST_INCLUDE_DIRS})

add_executable(${TEST_NAME} test_node.cpp)

# Link to our static library to Google Test libraries
target_link_libraries(${TEST_NAME}
                      ${PROJECT_NAME}
                      ${GTEST_LIBRARIES})

add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
