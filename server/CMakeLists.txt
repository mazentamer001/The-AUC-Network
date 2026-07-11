cmake_minimum_required(VERSION 3.21)
project(Server LANGUAGES CXX)

find_package(Boost REQUIRED)
find_package(SQLite3 REQUIRED)

if(NOT TARGET Shared)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared ${CMAKE_CURRENT_BINARY_DIR}/shared)
endif()

add_executable(Server
    src/main.cpp
    networking/Server.cpp
    networking/Session.cpp
    networking/Dispatcher.cpp
    services/AuthService.cpp
    services/RegistrationService.cpp
    services/ProfileService.cpp
    services/ChatService.cpp
    services/MarketplaceService.cpp
    services/ForumService.cpp
    services/FileStorageService.cpp
    store/InMemoryStore.cpp
    store/Database.cpp
)

target_include_directories(Server PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/store
    ${CMAKE_CURRENT_SOURCE_DIR}/networking
    ${CMAKE_CURRENT_SOURCE_DIR}/models
)

target_link_libraries(Server PRIVATE Shared SQLite3::SQLite3)