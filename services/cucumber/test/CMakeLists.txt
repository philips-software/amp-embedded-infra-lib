add_executable(services.cucumber_test)
emil_build_for(services.cucumber_test PREREQUISITE_BOOL EMIL_BUILD_TESTS)
emil_add_test(services.cucumber_test)

target_link_libraries(services.cucumber_test PUBLIC
    gmock_main
    hal.interfaces_test_doubles
    hal.synchronous_interfaces_test_doubles
    infra.timer
    infra.timer_test_helper
    services.cucumber
    services.network
    services.network_test_doubles
    services.util
)

target_sources(services.cucumber_test PRIVATE
    TestCucumberWireProtocolParser.cpp
    TestCucumberWireProtocolServer.cpp
)
