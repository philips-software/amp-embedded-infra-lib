add_executable(hal.synchronous_test_doubles_test)
add_test(NAME hal.synchronous_test_doubles_test COMMAND hal.synchronous_test_doubles_test)

target_link_libraries(hal.synchronous_test_doubles_test
    PUBLIC
    hal.synchronous_test_doubles

    gtest
    gmock
    gmock_main
)

target_sources(hal.synchronous_test_doubles_test
    PRIVATE
    TestSynchronousFixedRandomDataGenerator.cpp
)
