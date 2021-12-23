add_library(services.util EXCLUDE_FROM_ALL STATIC)
add_library(emil::services.util ALIAS services.util)
install(TARGETS services.util EXPORT emilServicesTargets)

target_link_libraries(services.util PUBLIC
    hal.interfaces
    infra.util
    infra.stream
    infra.syntax
    crypto.mbedtls
)

target_sources(services.util PRIVATE
    ConfigurationStore.cpp
    ConfigurationStore.hpp
    CyclicStore.cpp
    CyclicStore.hpp
    DebouncedButton.cpp
    DebouncedButton.hpp
    DebugLed.cpp
    DebugLed.hpp
    FlashAlign.cpp
    FlashAlign.hpp
    FlashDelayedInit.cpp
    FlashDelayedInit.hpp
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

if (BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(test_doubles)
endif()
