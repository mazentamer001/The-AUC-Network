cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
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
target_include_directories(build001 PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)
target_link_libraries(build001 PRIVATE Qt6::Widgets Threads::Threads Boost::system Shared)

add_executable(TestGUI
    TestGUIMain.cpp
    TestWindow.cpp
    src/Client.cpp
)
target_link_libraries(TestGUI PRIVATE Qt6::Widgets Boost::system Threads::Threads Shared)
target_include_directories(TestGUI PRIVATE
    include                                       
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)
