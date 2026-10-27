cmake_minimum_required(VERSION 3.16)

project(Calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# 界面与计算逻辑放在静态库里，后续单元测试可以复用同一份代码。
qt_add_library(calculator_core STATIC
    calculator.cpp
    calculator.h
    calculator.ui
    resources.qrc
)
target_link_libraries(calculator_core PUBLIC Qt6::Widgets)
target_include_directories(calculator_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

qt_add_executable(Calculator WIN32
    main.cpp
)
target_link_libraries(Calculator PRIVATE calculator_core)
