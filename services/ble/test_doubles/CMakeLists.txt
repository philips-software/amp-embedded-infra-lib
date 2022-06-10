ccola_component(ble_test_doubles source)

    ccola_sources(
        BondStorageManagerMock.hpp
        GapPeripheralMock.hpp
    )

    ccola_dependencies(
        ble
    )

ccola_end_component()
