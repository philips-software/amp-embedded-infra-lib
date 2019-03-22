ccola_component(test_doubles_test gmock_test)

    ccola_sources(
        TestI2cRegisterAccessMock.cpp
    )

    ccola_dependencies(
        test_doubles
        infra.event_test_helper
    )

ccola_end_component()
