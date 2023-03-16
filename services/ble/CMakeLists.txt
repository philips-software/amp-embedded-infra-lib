ccola_component(ble static_library)

    ccola_sources(
        BondBlobPersistence.cpp
        BondBlobPersistence.hpp
        BondStorageSynchronizer.cpp
        BondStorageSynchronizer.hpp
        Gap.cpp
        Gap.hpp
        Gatt.cpp
        Gatt.hpp
        GattClient.cpp
        GattClient.hpp
        GattClientCharacteristicImpl.cpp
        GattClientCharacteristicImpl.hpp
        GattServer.cpp
        GattServer.hpp
        GattServerCharacteristicImpl.cpp
        GattServerCharacteristicImpl.hpp
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
