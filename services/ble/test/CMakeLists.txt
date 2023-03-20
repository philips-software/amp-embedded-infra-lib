ccola_component(ble_test gmock_test)

    ccola_sources(
        TestBondBlobPersistence.cpp
        TestBondStorageSynchronizer.cpp
        TestGapCentral.cpp
        TestGapPeripheral.cpp
        TestGatt.cpp
        TestGattClient.cpp
        TestGattServer.cpp
    )

    ccola_dependencies(
        ble
        ble_test_doubles
        services.services_test_doubles
    )

ccola_end_component()
