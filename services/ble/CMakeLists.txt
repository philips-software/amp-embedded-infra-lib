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
        GattServer.cpp
        GattServer.hpp
        GattCharacteristicImpl.cpp
        GattCharacteristicImpl.hpp
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
