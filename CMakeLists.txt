cmake_minimum_required(VERSION 3.20)
project(cake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(CAKE_OTHER_DIRECTORIES_DIRECTORY ${CMAKE_SOURCE_DIR}/packages/other_packages)

add_subdirectory(src)
