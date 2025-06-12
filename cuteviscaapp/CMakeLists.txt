cmake_minimum_required(VERSION 3.16)

project(cutevisca VERSION 1.0 LANGUAGES CXX)

#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network Mqtt)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(cuteviscaapp
  main.cpp
  cutemqttclient.h cutemqttclient.cpp
)

qt_add_qml_module(cuteviscaapp
    URI visca
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES videorouterclient.h videorouterclient.cpp
)

target_link_libraries(cuteviscaapp
    PRIVATE Qt6::Quick Qt6::Network Qt6::Mqtt libcutevisca
)

include(GNUInstallDirs)
install(TARGETS cuteviscaapp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
