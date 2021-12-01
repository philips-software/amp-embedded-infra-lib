add_executable(hal.interfaces_test)
add_test(NAME hal.interfaces_test COMMAND hal.interfaces_test)

target_link_libraries(hal.interfaces_test
    PUBLIC
    hal.interfaces
    hal.interfaces_test_doubles
    infra.event_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(hal.interfaces_test
    PRIVATE
    TestCan.cpp
    TestFlash.cpp
    TestI2cRegisterAccess.cpp
    TestMacAddress.cpp
    TestQuadSpi.cpp
)
