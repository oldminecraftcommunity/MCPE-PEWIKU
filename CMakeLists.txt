cmake_minimum_required(VERSION 3.16)

project(pewiku LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(handheld/project/linux)
add_subdirectory(handheld/project/dedicated_server)
