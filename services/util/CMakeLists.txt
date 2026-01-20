add_library(services.util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_cpp(services.util Flash.proto)
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
    EchoOnMessageCommunication.cpp
    EchoOnMessageCommunication.hpp
    EchoOnSesame.cpp
    EchoOnSesame.hpp
    EchoPolicyDiffieHellman.cpp
    EchoPolicyDiffieHellman.hpp
    EchoPolicySymmetricKey.cpp
    EchoPolicySymmetricKey.hpp
    FlashAlign.cpp
    FlashAlign.hpp
    FlashDelegate.hpp
    FlashEcho.cpp
    FlashEcho.hpp
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
    MessageCommunicationWindowed.cpp
    MessageCommunicationWindowed.hpp
    RepeatingButton.cpp
    RepeatingButton.hpp
    SerialCommunicationLoopback.cpp
    SerialCommunicationLoopback.hpp
    Sesame.hpp
    SesameCobs.cpp
    SesameCobs.hpp
    SesameCrypto.hpp
    SesameSecured.cpp
    SesameSecured.hpp
    SesameWindowed.cpp
    SesameWindowed.hpp
    Sha256.hpp
    SignalLed.cpp
    SignalLed.hpp
    SleepableFlashSpi.cpp
    SleepableFlashSpi.hpp
    SleepOnInactivityFlashDecorator.cpp
    SleepOnInactivityFlashDecorator.hpp
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

if (EMIL_HOST_BUILD)
    target_link_libraries(services.util PUBLIC
        hal.generic
    )
endif()

if (EMIL_INCLUDE_MBEDTLS OR NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
    if (NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
        target_link_libraries(services.util PUBLIC
            ${EMIL_EXTERNAL_MBEDTLS_TARGET}
        )
    else()
        target_link_libraries(services.util PUBLIC
            mbedcrypto
        )
    endif()

    target_compile_definitions(services.util PUBLIC
        EMIL_USE_MBEDTLS
    )

    target_sources(services.util PRIVATE
        AesMbedTls.cpp
        AesMbedTls.hpp
        EchoInstantiationSecured.cpp
        EchoInstantiationSecured.hpp
        MbedTlsRandomDataGeneratorWrapper.cpp
        MbedTlsRandomDataGeneratorWrapper.hpp
        SesameCryptoMbedTls.cpp
        SesameCryptoMbedTls.hpp
        Sha256MbedTls.cpp
        Sha256MbedTls.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
