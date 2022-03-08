add_executable(hal.interfaces_test)
gtest_discover_tests(hal.interfaces_test AUTO)

target_link_libraries(hal.interfaces_test PUBLIC
    gmock_main
    hal.interfaces
    hal.interfaces_test_doubles
    infra.event_test_helper
)

target_sources(hal.interfaces_test PRIVATE
    TestCan.cpp
    TestFlash.cpp
    TestI2cRegisterAccess.cpp
    TestMacAddress.cpp
    TestQuadSpi.cpp
)
