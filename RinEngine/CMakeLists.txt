cmake_minimum_required(VERSION 3.16)

project(RinEngine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Concurrent Multimedia MultimediaWidgets)
qt_standard_project_setup(REQUIRES 6.5)

add_subdirectory(src/core)
add_subdirectory(src/app)
