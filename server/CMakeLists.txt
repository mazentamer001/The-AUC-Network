cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)
add_executable(Serverbuild001 src/main.cpp networking/Server.cpp networking/Session.cpp)
target_include_directories(Serverbuild001 PUBLIC include)
