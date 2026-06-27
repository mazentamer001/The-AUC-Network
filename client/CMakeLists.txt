cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

if(NOT TARGET Shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
endif()

add_executable(build001
    src/main.cpp
    src/Client.cpp
)

target_include_directories(build001 PRIVATE include ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)
target_link_libraries(build001 PRIVATE Qt6::Widgets Threads::Threads Boost::system Shared)
