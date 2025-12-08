cmake_minimum_required(VERSION 3.16)
project(QtCloseConfirmExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
