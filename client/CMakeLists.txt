cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(build001 src/main.cpp)
target_link_libraries(build001 PRIVATE Qt6::Widgets)
