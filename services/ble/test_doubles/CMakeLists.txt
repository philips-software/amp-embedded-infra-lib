add_library(services.ble_test_doubles INTERFACE)
emil_build_for(services.ble_test_doubles BOOL BUILD_TESTING)

target_link_libraries(services.ble_test_doubles INTERFACE
    services.ble
    gmock
)

target_sources(services.ble_test_doubles PRIVATE
    BondStorageSynchronizerMock.hpp
    GapCentralMock.hpp
    GapCentralObserverMock.hpp
    GapPeripheralMock.hpp
    GapBondingMock.hpp
    GapPairingMock.hpp
    GapPairingObserverMock.hpp
    GapPeripheralObserverMock.hpp
    GapBondingObserverMock.hpp
    GattClientMock.hpp
    GattServerMock.hpp
)
