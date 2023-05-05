add_library(hal.synchronous_interfaces_test_doubles STATIC)
emil_build_for(hal.synchronous_interfaces_test_doubles BOOL BUILD_TESTING)

target_link_libraries(hal.synchronous_interfaces_test_doubles PUBLIC
    hal.synchronous_interfaces
    gmock
)

target_sources(hal.synchronous_interfaces_test_doubles PRIVATE
    SynchronousEncryptorMock.hpp
    SynchronousFixedRandomDataGenerator.cpp
    SynchronousFixedRandomDataGenerator.hpp
    SynchronousFlashStub.cpp
    SynchronousFlashStub.hpp
    SynchronousRandomDataGeneratorMock.hpp
    SynchronousSerialCommunicationMock.cpp
    SynchronousSerialCommunicationMock.hpp
    SynchronousSpiMock.cpp
    SynchronousSpiMock.hpp
    TimeKeeperMock.hpp
)

add_subdirectory(test)
