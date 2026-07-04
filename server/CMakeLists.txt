cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS system)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/src/Message.cpp
)

target_include_directories(Server PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/store
    ${CMAKE_CURRENT_SOURCE_DIR}/networking
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/include
)

target_link_libraries(Server PRIVATE Boost::system Shared)
