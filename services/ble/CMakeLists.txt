ccola_component(ble static_library)

    ccola_sources(
        BondStorageManager.cpp
        BondStorageManager.hpp
        Gap.hpp
        Gatt.cpp
        Gatt.hpp
        GattCharacteristicImpl.cpp
        GattCharacteristicImpl.hpp
    )

    ccola_dependencies(
        hal.interfaces
        infra.util
    )

ccola_end_component()

ccola_subdirectories(
    test
)
