ccola_component(ble_test_doubles x86: source other: noncompile)

    ccola_sources(
        BondStorageSynchronizerMock.hpp
        GapCentralMock.hpp
        GapCentralObserverMock.hpp
        GapPeripheralMock.hpp
        GapPeripheralBondingMock.hpp
        GapPeripheralPairingMock.hpp
        GapPeripheralObserverMock.hpp
        GapPeripheralBondingObserverMock.hpp
        GattClientMock.hpp
        GattServerMock.hpp
    )

    ccola_dependencies(
        ble
        gmock.gmock
    )

ccola_end_component()
