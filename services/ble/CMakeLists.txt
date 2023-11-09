add_library(services.ble ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_cpp(services.ble Gap.proto)
protocol_buffer_csharp(services.ble Gap.proto)
protocol_buffer_java(services.ble Gap.proto)

protocol_buffer_echo_cpp(services.ble Gatt.proto)
protocol_buffer_csharp(services.ble Gatt.proto)
protocol_buffer_java(services.ble Gatt.proto)

target_link_libraries(services.ble PUBLIC
    hal.interfaces
    infra.util
)

target_sources(services.ble PRIVATE
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
    GattServerCharacteristicImpl.cpp
    GattServerCharacteristicImpl.hpp
)

add_subdirectory(test)
add_subdirectory(test_doubles)
