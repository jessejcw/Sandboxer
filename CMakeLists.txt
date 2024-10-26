cmake_minimum_required(VERSION 3.15)
project(Sandboxer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(runner)
add_subdirectory(csv_to_sqlite)

