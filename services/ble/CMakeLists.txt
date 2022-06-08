ccola_component(ble static_library)

    ccola_sources(
        Gap.hpp
        Gatt.cpp
        Gatt.hpp
        GattCharacteristicImpl.cpp
        GattCharacteristicImpl.hpp
        Uuids.hpp
    )

    ccola_dependencies(
        infra.util
    )

ccola_end_component()

ccola_subdirectories(
    test
)
