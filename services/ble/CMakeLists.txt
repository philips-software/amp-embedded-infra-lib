add_library(services.ble ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_all(services.ble Gap.proto)
protocol_buffer_csharp(services.ble Gap.proto)
protocol_buffer_java(services.ble Gap.proto)

protocol_buffer_echo_all(services.ble Gatt.proto)
protocol_buffer_csharp(services.ble Gatt.proto)
protocol_buffer_java(services.ble Gatt.proto)

protocol_buffer_echo_all(services.ble Dtm.proto)
protocol_buffer_csharp(services.ble Dtm.proto)
protocol_buffer_java(services.ble Dtm.proto)

target_link_libraries(services.ble PUBLIC
    hal.interfaces
    infra.util
    services.tracer
)

target_sources(services.ble PRIVATE
    Att.hpp
    BondBlobPersistence.cpp
    BondBlobPersistence.hpp
    BondStorageSynchronizer.cpp
    BondStorageSynchronizer.hpp
    Gap.cpp
    Gap.hpp
    GapPeripheralIntervalDecorator.cpp
    GapPeripheralIntervalDecorator.hpp
    Gatt.cpp
    Gatt.hpp
    GattClient.cpp
    GattClient.hpp
    ClaimingGattClientAdapter.cpp
    ClaimingGattClientAdapter.hpp
    GattServer.cpp
    GattServer.hpp
    GattServerCharacteristicImpl.cpp
    GattServerCharacteristicImpl.hpp
    RetryGattClientCharacteristicsOperations.cpp
    RetryGattClientCharacteristicsOperations.hpp
)

add_subdirectory(test)
add_subdirectory(test_doubles)
