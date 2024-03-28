add_library(hal.interfaces ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_include_directories(hal.interfaces PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(hal.interfaces PUBLIC
    infra.event
    $<$<BOOL:${EMIL_BUILD_UNIX}>:stdc++fs>
)

target_sources(hal.interfaces PRIVATE
    AnalogToDigitalPin.hpp
    AsyncGpio.cpp
    AsyncGpio.hpp
    BackupRam.hpp
    Can.cpp
    Can.hpp
    CommunicationConfigurator.hpp
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
    SerialCommunication.cpp
    SerialCommunication.hpp
    Spi.cpp
    Spi.hpp
    UsbCustomHid.hpp
    UsbLinkLayer.hpp
    $<$<BOOL:${EMIL_HOST_BUILD}>:FileSystem.cpp>
    $<$<BOOL:${EMIL_HOST_BUILD}>:FileSystem.hpp>
)

add_subdirectory(test)
add_subdirectory(test_doubles)
