add_library(hal.interfaces ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS hal.interfaces EXPORT emilHalTargets)

target_include_directories(hal.interfaces PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(hal.interfaces PUBLIC
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

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
add_subdirectory(test_doubles)
