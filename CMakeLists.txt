cmake_minimum_required(VERSION 3.16)

project(cutevisca VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.8)

add_subdirectory(libcutevisca)
add_subdirectory(cuteviscaapp)
