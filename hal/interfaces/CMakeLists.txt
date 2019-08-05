ccola_component(interfaces static_library)

    ccola_sources(
        AnalogToDigitalPin.hpp
        CommunicationConfigurator.hpp
        DigitalToAnalogPin.hpp
        Ethernet.cpp
        Ethernet.hpp
    win:
        FileSystem.cpp
        FileSystem.hpp
    all:
        Flash.cpp
        Flash.hpp
        FlashHomogeneous.cpp
        FlashHomogeneous.hpp
        FlashId.hpp
        FlashStatelessStub.cpp
        FlashStatelessStub.hpp
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

ccola_end_component()

ccola_subdirectories(
    test_doubles
    test
)
