add_library(services.util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_cpp(services.util SesameSecurity.proto)

target_link_libraries(services.util PUBLIC
    hal.interfaces
    infra.syntax
    infra.timer
    protobuf.echo
    services.tracer
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
    EchoOnSesame.cpp
    EchoOnSesame.hpp
    EchoOnSesameSymmetricKey.cpp
    EchoOnSesameSymmetricKey.hpp
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
    RepeatingButton.cpp
    RepeatingButton.hpp
    SerialCommunicationLoopback.cpp
    SerialCommunicationLoopback.hpp
    Sesame.hpp
    SesameCobs.cpp
    SesameCobs.hpp
    SesameSecured.cpp
    SesameSecured.hpp
    SesameWindowed.cpp
    SesameWindowed.hpp
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
    TracingEchoOnSesame.hpp
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
