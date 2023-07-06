add_executable(services.ble_test)
emil_build_for(services.ble_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.ble_test)

target_link_libraries(services.ble_test PUBLIC
    gmock_main
    services.ble
    services.ble_test_doubles
)

target_sources(services.ble_test PRIVATE
    TestBondBlobPersistence.cpp
    TestBondStorageSynchronizer.cpp
    TestGapCentral.cpp
    TestGapPairing.cpp
    TestGapPeripheral.cpp
    TestGatt.cpp
    TestGattClient.cpp
    TestGattServer.cpp
)
