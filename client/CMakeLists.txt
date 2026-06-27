cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)

add_executable(build001
    src/main.cpp
    src/Client.cpp
)

target_link_libraries(build001 PRIVATE Qt6::Widgets Threads::Threads)
target_include_directories(build001 PRIVATE include)
