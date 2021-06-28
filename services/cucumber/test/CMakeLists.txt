ccola_component(cucumber_test gmock_test)

    ccola_sources(
        TestCucumberWireProtocolParser.cpp
        TestCucumberWireProtocolServer.cpp
    )

    ccola_dependencies(
        network
        network_test_doubles
        infra.timer
        infra.timer_test_helper
        infra.util_test_helper
        hal.test_doubles
        hal.synchronous_test_doubles
        services.util
        services.cucumber
    )

ccola_end_component()
