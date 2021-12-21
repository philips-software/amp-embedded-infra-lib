add_library(hal.synchronous_interfaces_test_doubles EXCLUDE_FROM_ALL STATIC)

target_link_libraries(hal.synchronous_interfaces_test_doubles
    PUBLIC
    hal.synchronous_interfaces
    gmock
)

target_sources(hal.synchronous_interfaces_test_doubles
    PRIVATE
    TimeKeeperMock.hpp
    SynchronousSerialCommunicationMock.hpp
    SynchronousSerialCommunicationMock.cpp
    SynchronousRandomDataGeneratorMock.hpp
    SynchronousFlashStub.hpp
    SynchronousFlashStub.cpp
    SynchronousFixedRandomDataGenerator.hpp
    SynchronousFixedRandomDataGenerator.cpp
    SynchronousEncryptorMock.hpp
)

add_subdirectory(test)
