cmake_minimum_required(VERSION 3.5)

project(haifisch_test LANGUAGES CXX)

include_directories(../haifisch)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS -ftree-vectorize)

set(SOURCES main.cpp)
set(HEADERS tests.hpp)

add_executable(haifisch_test ${SOURCES} ${HEADERS})
target_link_libraries(haifisch_test -fopenmp -lgtest)
