add_library(services.util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_cpp(services.util MessageCommunicationSecurity.proto)

target_link_libraries(services.util PUBLIC
    hal.interfaces
    infra.syntax
    infra.timer
    protobuf.echo
)

target_sources(services.util PRIVATE
    Aes.hpp
    ConfigurationStore.cpp
    ConfigurationStore.hpp
    CyclicStore.cpp
    CyclicStore.hpp
    DebouncedButton.cpp
    DebouncedButton.hpp
    DebugLed.cpp
    DebugLed.hpp
    EchoInstantiation.cpp
    EchoInstantiation.hpp
    EchoOnMessageCommunication.cpp
    EchoOnMessageCommunication.hpp
    EchoOnMessageCommunicationSymmetricKey.cpp
    EchoOnMessageCommunicationSymmetricKey.hpp
    FlashAlign.cpp
    FlashAlign.hpp
    FlashMultipleAccess.cpp
    FlashMultipleAccess.hpp
    FlashQuadSpi.cpp
    FlashQuadSpi.hpp
    FlashQuadSpiCypressFll.cpp
    FlashQuadSpiCypressFll.hpp
    FlashQuadSpiMicronN25q.cpp
    FlashQuadSpiMicronN25q.hpp
    FlashQuadSpiSingleSpeed.cpp
    FlashQuadSpiSingleSpeed.hpp
    FlashSpi.cpp
    FlashSpi.hpp
    FlashRegion.cpp
    FlashRegion.hpp
    GpioPinInverted.cpp
    GpioPinInverted.hpp
    I2cMultipleAccess.cpp
    I2cMultipleAccess.hpp
    LowPowerSerialCommunication.cpp
    LowPowerSerialCommunication.hpp
    LowPowerSpiMaster.cpp
    LowPowerSpiMaster.hpp
    MessageCommunication.hpp
    MessageCommunicationCobs.cpp
    MessageCommunicationCobs.hpp
    MessageCommunicationSecured.cpp
    MessageCommunicationSecured.hpp
    MessageCommunicationWindowed.cpp
    MessageCommunicationWindowed.hpp
    RepeatingButton.cpp
    RepeatingButton.hpp
    SerialCommunicationLoopback.cpp
    SerialCommunicationLoopback.hpp
    Sha256.hpp
    SignalLed.cpp
    SignalLed.hpp
    SpiMasterWithChipSelect.cpp
    SpiMasterWithChipSelect.hpp
    SpiMultipleAccess.cpp
    SpiMultipleAccess.hpp
    Stoppable.hpp
    Terminal.cpp
    Terminal.hpp
    TimeWithLocalization.cpp
    TimeWithLocalization.hpp
    WritableConfiguration.hpp
)

if (EMIL_INCLUDE_MBEDTLS)
    target_link_libraries(services.util PUBLIC
        mbedcrypto
    )

    target_sources(services.util PRIVATE
        AesMbedTls.cpp
        AesMbedTls.hpp
        Sha256MbedTls.cpp
        Sha256MbedTls.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
