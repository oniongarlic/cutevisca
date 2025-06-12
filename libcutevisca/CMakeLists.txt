cmake_minimum_required(VERSION 3.16)

project(cutevisca VERSION 1.0 LANGUAGES CXX)

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_library(libcutevisca SHARED
  viscaudpcontroller.h viscaudpcontroller.cpp
  viscaemulator.h viscaemulator.cpp
)

target_link_libraries(libcutevisca
    PRIVATE Qt6::Core Qt6::Network
)

target_include_directories(libcutevisca INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

#include(GNUInstallDirs)
#install(TARGETS libcutevisca
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
