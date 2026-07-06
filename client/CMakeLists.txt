cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(NOT TARGET Shared)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
endif()

# ── Client as a static lib so it's compiled once ──────────────────────────────
add_library(ClientLib STATIC src/Client.cpp)
set_target_properties(ClientLib PROPERTIES AUTOMOC OFF)
target_include_directories(ClientLib PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)
target_link_libraries(ClientLib PUBLIC Boost::system Threads::Threads Shared)

#terminal client
add_executable(build001 src/main.cpp)
set_target_properties(build001 PROPERTIES AUTOMOC OFF)
target_link_libraries(build001 PRIVATE ClientLib)

#GUI client
add_executable(TestGUI
    TestGUIMain.cpp
    ui/MainWindow.cpp
    ui/pages/HomePage.cpp
    ui/pages/RegisterPage.cpp
    ui/pages/LoginPage.cpp
    ui/pages/MainShell.cpp
    ui/panels/ChatPanel.cpp
    ui/panels/MarketplacePanel.cpp
    ui/panels/FilesPanel.cpp
    ui/panels/FilesPanel.h
    ui/panels/ForumPanel.cpp 
    ui/panels/ProfilePanel.cpp 
)
set_target_properties(TestGUI PROPERTIES AUTOMOC ON)
target_include_directories(TestGUI PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)
target_link_libraries(TestGUI PRIVATE Qt6::Widgets ClientLib)