cmake_minimum_required(VERSION 3.16)
project(YuvViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets Concurrent Test)

qt_add_executable(YuvViewer
    src/main.cpp
    src/core/YuvParser.cpp
    src/core/YuvToRgbConverter.cpp
    src/core/RulerRenderer.cpp
    src/ui/MainWindow.cpp
    src/ui/ImageViewer.cpp
    src/ui/OpenYuvDialog.cpp
    src/ui/CoordinateInputPanel.cpp
)

target_sources(YuvViewer PRIVATE res/app.rc)

target_include_directories(YuvViewer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/ui
)

target_link_libraries(YuvViewer PRIVATE Qt6::Widgets Qt6::Concurrent)

set_target_properties(YuvViewer PROPERTIES
    WIN32_EXECUTABLE ON
)

# --- Unit tests ---

enable_testing()

set(TEST_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

qt_add_executable(tst_YuvParser
    test/tst_YuvParser.cpp
    src/core/YuvParser.cpp
)

target_include_directories(tst_YuvParser PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_YuvParser PRIVATE Qt6::Test Qt6::Core)
add_test(NAME tst_YuvParser COMMAND tst_YuvParser)

qt_add_executable(tst_YuvToRgbConverter
    test/tst_YuvToRgbConverter.cpp
    src/core/YuvToRgbConverter.cpp
)

target_include_directories(tst_YuvToRgbConverter PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(tst_YuvToRgbConverter PRIVATE Qt6::Test Qt6::Core Qt6::Gui)
add_test(NAME tst_YuvToRgbConverter COMMAND tst_YuvToRgbConverter)
