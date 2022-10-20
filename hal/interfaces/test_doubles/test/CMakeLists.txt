add_executable(hal.interfaces_test_doubles_test)
emil_add_test(hal.interfaces_test_doubles_test)

target_link_libraries(hal.interfaces_test_doubles_test PUBLIC
    gmock_main
    hal.interfaces_test_doubles
    infra.event_test_helper
)

target_sources(hal.interfaces_test_doubles_test PRIVATE
    TestI2cRegisterAccessMock.cpp
)

emil_clangformat_target(hal.interfaces_test_doubles_test)
