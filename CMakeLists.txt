# 8-14-2024 Initial Commit
cmake_minimum_required(VERSION 3.5.0)
project(BalatroCV VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(BalatroCV
                constants.cpp
                main.cpp
                cards.cpp
                rankSuitIsolator.cpp
                videoStream.cpp
)

target_link_libraries(BalatroCV ${OpenCV_LIBS})

include(CTest)
enable_testing()

