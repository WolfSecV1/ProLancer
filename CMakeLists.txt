cmake_minimum_required(VERSION 3.16)

project(Lancer LANGUAGES CXX)

configure_file(assets/version.txt version.txt COPYONLY)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(OpenGL REQUIRED)

find_package(Qt6 
    REQUIRED 
    COMPONENTS 
    Core 
    Gui 
    Widgets 
    OpenGL 
    OpenGLWidgets
)

add_executable(Lancer)

target_sources(Lancer PRIVATE
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/Canvas.cpp
    src/core/CanvasController.cpp
    src/fileReader.cpp
    src/core/CanvasController.h
    src/data/StrokePoint.h
    src/core/math/mathUtils.h
    src/core/math/mathUtils.cpp
    src/core/StrokeProcessor.h 
    src/core/StrokeProcessor.cpp 
    src/rendering/StrokeRenderer.h 
    src/rendering/StrokeRenderer.cpp
    src/core/StrokeManager.h
    src/core/StrokeManager.cpp
    src/ui/tools/HSVColorPicker.h
    src/ui/tools/HSVColorPicker.cpp
    resources.qrc
)

target_include_directories(Lancer PRIVATE
    src
    src/core
    src/core/math
    src/data 
    src/renderer
    src/ui
)

target_link_libraries(Lancer 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
    OpenGL::GL
)

qt_add_resources(MyApp "resources"
    FILES resources.qrc
)

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
