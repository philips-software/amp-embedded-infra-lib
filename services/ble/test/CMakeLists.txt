ccola_component(ble_test gmock_test)

    ccola_sources(
        TestBondStorageManager.cpp
        TestGatt.cpp
    )

    ccola_dependencies(
        ble
    )

ccola_end_component()
