ccola_component(synchronous_test_doubles x86: static_library other: noncompile)

    ccola_sources(
        SynchronousEncryptorMock.hpp
        SynchronousFixedRandomDataGenerator.cpp
        SynchronousFixedRandomDataGenerator.hpp
        SynchronousFlashStub.cpp
        SynchronousFlashStub.hpp
        SynchronousRandomDataGeneratorMock.hpp
        SynchronousSerialCommunicationMock.cpp
        SynchronousSerialCommunicationMock.hpp
        TimeKeeperMock.hpp
    )

    ccola_dependencies(
        synchronous_interfaces
        gmock.gmock
    )

ccola_end_component()

ccola_subdirectories(
    test
)
