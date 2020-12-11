ccola_component(interfaces static_library)

    ccola_sources(
    x86:
        FileSystem.cpp
        FileSystem.hpp
    all:
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
    )

    ccola_dependencies(
        infra.util
        infra.event
    )

    ccola_external_libraries(
    linux:
        stdc++fs
    )

ccola_end_component()

ccola_subdirectories(
    test_doubles
    test
)
