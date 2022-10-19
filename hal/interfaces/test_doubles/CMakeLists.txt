add_library(hal.interfaces_test_doubles ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(hal.interfaces_test_doubles PRIVATE
    hal.interfaces
    infra.stream
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

emil_clangformat_target(hal.interfaces_test_doubles)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
