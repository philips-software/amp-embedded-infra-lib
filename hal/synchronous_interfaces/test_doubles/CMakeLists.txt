add_library(hal.synchronous_interfaces_test_doubles ${EMIL_EXCLUDE_FROM_ALL} STATIC)

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

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
