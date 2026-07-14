cmake_minimum_required(VERSION 3.21)
project(Shared LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core)

find_package(nlohmann_json 3.2.0 QUIET)
if(NOT TARGET nlohmann_json::nlohmann_json)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

add_library(Shared STATIC src/Message.cpp)

target_include_directories(Shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Shared PUBLIC Qt6::Core nlohmann_json::nlohmann_json)