cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(Shared STATIC src/User.cpp src/Message.cpp)
target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Shared PUBLIC Qt6::Core)
