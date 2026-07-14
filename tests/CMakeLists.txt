find_package(GTest REQUIRED)

add_executable(message_tests
    test_message.cpp
)

target_link_libraries(message_tests
    PRIVATE
    GTest::gtest_main
    Shared
)

include(GoogleTest)
gtest_discover_tests(message_tests)
