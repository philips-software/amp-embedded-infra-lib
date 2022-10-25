ccola_component(ble_test gmock_test)

    ccola_sources(
        TestBondStorageManager.cpp
        TestGatt.cpp
        TestPersistingBondStorage.cpp
    )

    ccola_dependencies(
        ble
        ble_test_doubles
        services.services_test_doubles
    )

ccola_end_component()
