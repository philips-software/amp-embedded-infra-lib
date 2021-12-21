add_executable(services.cucumber_test)
gtest_discover_tests(services.cucumber_test AUTO)

target_link_libraries(services.cucumber_test
    PUBLIC
    hal.interfaces_test_doubles
    hal.synchronous_test_doubles
    infra.timer
    infra.timer_test_helper
    services.cucumber
    services.network
    services.network_test_doubles
    services.util

    gtest
    gmock
    gmock_main
)

target_sources(services.cucumber_test
    PRIVATE
    TestCucumberWireProtocolParser.cpp
    TestCucumberWireProtocolServer.cpp
)
