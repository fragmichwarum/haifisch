cmake_minimum_required(VERSION 3.5)

project(haifisch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-O3 -ftree-vectorize -march=native -mtune=generic -fopenmp -fsanitize=leak")

set(SOURCES main.cpp)
set(HEADERS haifisch/matrix.hpp util/func_benchmark.hpp util/logger.cpp util/logger.hpp)

add_executable(haifisch ${SOURCES} ${HEADERS})
target_link_libraries(haifisch -fopenmp gtest pthread)
