cmake_minimum_required(VERSION 3.16)

project(Calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Test)

# 界面与计算逻辑放在静态库里，应用程序和单元测试共用同一份代码。
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

enable_testing()

qt_add_executable(tst_calculator
    tests/tst_calculator.cpp
)
target_link_libraries(tst_calculator PRIVATE calculator_core Qt6::Test)
add_test(NAME calculator COMMAND tst_calculator)
set_tests_properties(calculator PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

# 用于生成实验报告截图的辅助工具，不影响计算器本身。
qt_add_executable(capture_screenshots
    tools/capture_screenshots.cpp
)
target_link_libraries(capture_screenshots PRIVATE calculator_core)
