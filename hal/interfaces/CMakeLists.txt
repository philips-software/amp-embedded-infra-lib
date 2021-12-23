add_library(hal.interfaces EXCLUDE_FROM_ALL STATIC)
add_library(emil::hal.interfaces ALIAS hal.interfaces)
install(TARGETS hal.interfaces EXPORT emilHalTargets)

target_include_directories(hal.interfaces PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(hal.interfaces PUBLIC
    infra.util
    infra.event
    $<$<BOOL:${TARGET_BUILD_UNIX}>:stdc++fs>
)

target_sources(hal.interfaces PRIVATE
    AnalogToDigitalPin.hpp
    AsyncGpio.cpp
    AsyncGpio.hpp
    Can.cpp
    Can.hpp
    CommunicationConfigurator.hpp
    Dac.hpp
    DigitalToAnalogPin.hpp
    Ethernet.hpp
    Flash.cpp
    Flash.hpp
    FlashHomogeneous.cpp
    FlashHomogeneous.hpp
    FlashId.hpp
    Gpio.cpp
    Gpio.hpp
    I2c.cpp
    I2c.hpp
    I2cRegisterAccess.cpp
    I2cRegisterAccess.hpp
    MacAddress.cpp
    MacAddress.hpp
    QuadSpi.cpp
    QuadSpi.hpp
    RandomDataGenerator.hpp
    Reset.hpp
    SerialCommunication.hpp
    Spi.cpp
    Spi.hpp
    UsbCustomHid.hpp
    UsbLinkLayer.hpp
    $<$<BOOL:${TARGET_ARCH_X86}>:FileSystem.cpp>
    $<$<BOOL:${TARGET_ARCH_X86}>:FileSystem.hpp>
)

if (BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(test_doubles)
endif()
