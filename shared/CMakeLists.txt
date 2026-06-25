cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)
add_executable(Sharedbuild001 src/User.cpp src/Message.cpp)
target_include_directories(Sharedbuild001 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
