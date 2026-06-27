cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)

if(NOT TARGET Shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
endif()

add_executable(Serverbuild001
    src/main.cpp
    networking/Server.cpp
    networking/Session.cpp
)

target_include_directories(Serverbuild001 PRIVATE include ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)
target_link_libraries(Serverbuild001 PRIVATE Shared)
