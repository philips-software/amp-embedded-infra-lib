add_executable(services.ble_test)
emil_add_test(services.ble_test)

target_link_libraries(services.ble_test PUBLIC
    gmock_main
    services.ble
    services.ble_test_doubles
)

target_sources(services.ble_test PRIVATE
    TestBondBlobPersistence.cpp
    TestBondStorageSynchronizer.cpp
        TestGap.cpp
    TestGatt.cpp
)

emil_clangformat_target(services.ble_test)
