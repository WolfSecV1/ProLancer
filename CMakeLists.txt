cmake_minimum_required(VERSION 3.16)
project(ProLancer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)

add_executable(ProLancer
    main.cpp
    MainWindow.cpp
    Canvas.cpp
    MainWindow.h
    Canvas.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)

target_link_libraries(ProLancer Qt6::Core Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
