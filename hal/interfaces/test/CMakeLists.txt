ccola_component(interfaces_test gmock_test)

    ccola_sources(
        TestFlash.cpp
        TestI2cRegisterAccess.cpp
        TestMacAddress.cpp
        TestQuadSpi.cpp
    )

    ccola_dependencies(
        interfaces
        test_doubles
        infra.event_test_helper
    )

ccola_end_component()
