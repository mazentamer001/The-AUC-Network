cmake_minimum_required(VERSION 3.21)
project(AUCNetworking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# ── SQLiteCpp ─────────────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG        3.3.1
)
FetchContent_MakeAvailable(SQLiteCpp)

# ── Shared ────────────────────────────────────────────────────────────────────
if(NOT TARGET Shared)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared
                 ${CMAKE_CURRENT_BINARY_DIR}/shared)
endif()

# ── Server ────────────────────────────────────────────────────────────────────
add_executable(Server
    src/main.cpp
    networking/Server.cpp
    networking/Session.cpp
    networking/Dispatcher.cpp
    store/Database.cpp
    store/InMemoryStore.cpp 
    services/AuthService.cpp
    services/RegistrationService.cpp
    services/ProfileService.cpp
    services/ChatService.cpp
    services/MarketplaceService.cpp
    services/ForumService.cpp
    services/FileStorageService.cpp
    services/OpportunityService.cpp
    services/AIService.cpp
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

target_link_libraries(Server PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    SQLiteCpp
    Shared
)