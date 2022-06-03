ccola_component(ble_test gmock_test)

    ccola_sources(
        TestBondStorageManager.cpp
    )

    ccola_dependencies(
        services.ble
    )

ccola_end_component()
