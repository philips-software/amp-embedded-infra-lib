add_library(services.ble_test_doubles ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_link_libraries(services.ble_test_doubles INTERFACE
    services.ble
    gmock
)

target_sources(services.ble_test_doubles PRIVATE
    BondStorageManagerMock.hpp
    GapPeripheralMock.hpp
    GattMock.hpp
)

emil_clangformat_target(services.ble_test_doubles)
