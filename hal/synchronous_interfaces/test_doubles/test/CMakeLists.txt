add_executable(hal.synchronous_interfaces_test_doubles_test)
gtest_discover_tests(hal.synchronous_interfaces_test_doubles_test AUTO)

target_link_libraries(hal.synchronous_interfaces_test_doubles_test
    PUBLIC
    hal.synchronous_interfaces_test_doubles

    gtest
    gmock
    gmock_main
)

target_sources(hal.synchronous_interfaces_test_doubles_test
    PRIVATE
    TestSynchronousFixedRandomDataGenerator.cpp
)
