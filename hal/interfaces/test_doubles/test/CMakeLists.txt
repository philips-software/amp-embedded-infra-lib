add_executable(hal.interfaces_test_doubles_test)
add_test(NAME hal.interfaces_test_doubles_test COMMAND hal.interfaces_test_doubles_test)

target_link_libraries(hal.interfaces_test_doubles_test
    PUBLIC
    hal.interfaces_test_doubles
    infra.event_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(hal.interfaces_test_doubles_test
    PRIVATE
    TestI2cRegisterAccessMock.cpp
)
