ccola_component(test_doubles x86: static_library other: noncompile)

    ccola_sources(
        AnalogToDigitalPinImplMock.hpp
        CommunicationConfiguratorMock.hpp
        DigitalToAnalogPinImplMock.hpp
        FileSystemStub.cpp
        FileSystemStub.hpp
        FlashMock.cpp
        FlashMock.hpp
        FlashStub.cpp
        FlashStub.hpp
        GpioMock.hpp
        GpioStub.cpp
        GpioStub.hpp
        I2cMock.cpp
        I2cMock.hpp
        I2cRegisterAccessMock.cpp
        I2cRegisterAccessMock.hpp
        QuadSpiStub.cpp
        QuadSpiStub.hpp
        RandomDataGeneratorMock.hpp
        ResetMock.hpp
        SerialCommunicationMock.cpp
        SerialCommunicationMock.hpp
        SerialCommunicationStub.cpp
        SerialCommunicationStub.hpp
        SpiMock.cpp
        SpiMock.hpp
    )

    ccola_dependencies(
        interfaces
        gmock.gmock
        infra.stream
    )

ccola_end_component()

ccola_subdirectories(
    test
)
