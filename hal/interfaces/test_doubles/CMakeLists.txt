add_library(hal.interfaces_test_doubles STATIC)
emil_build_for(hal.interfaces_test_doubles PREREQUISITE_BOOL BUILD_TESTING)

target_link_libraries(hal.interfaces_test_doubles PRIVATE
    hal.interfaces
    infra.stream
    infra.timer
    gmock
)

target_sources(hal.interfaces_test_doubles PRIVATE
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

add_subdirectory(test)
