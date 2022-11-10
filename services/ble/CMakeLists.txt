ccola_component(ble static_library)

    ccola_sources(
        BondBlobPersistence.cpp
        BondBlobPersistence.hpp
        BondStorageSynchronizer.cpp
        BondStorageSynchronizer.hpp
        Gap.hpp
        Gatt.cpp
        Gatt.hpp
        GattCharacteristicImpl.cpp
        GattCharacteristicImpl.hpp
        Uuids.hpp
    )

    ccola_dependencies(
        hal.interfaces
        infra.util
    )

ccola_end_component()

ccola_subdirectories(
    test_doubles
    test
)
