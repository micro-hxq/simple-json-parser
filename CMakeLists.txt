cmake_minimum_required(VERSION 3.6)
project(JsonParser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(TEST_FILES unittest.cpp JsonParser.cpp)

add_executable(parser_test ${TEST_FILES})
