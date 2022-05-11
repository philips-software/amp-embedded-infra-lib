ccola_component(synchronous_util_test gmock_test)

    ccola_sources(
        TestSynchronousFlashSpi.cpp
    )

    ccola_dependencies(
        synchronous_util
        hal.synchronous_test_doubles
    )

ccola_end_component()
