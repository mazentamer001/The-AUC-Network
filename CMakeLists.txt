cmake_minimum_required(VERSION 3.21)
project(AUCNetworking)

add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client)

enable_testing()

add_subdirectory(tests)

