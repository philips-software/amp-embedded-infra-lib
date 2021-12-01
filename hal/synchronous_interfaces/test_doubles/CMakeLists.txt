
add_library(hal.synchronous_test_doubles EXCLUDE_FROM_ALL STATIC)

target_link_libraries(hal.synchronous_test_doubles
    PUBLIC
    hal.synchronous_interfaces
    gmock
)

target_sources(hal.synchronous_test_doubles
    PRIVATE
    SynchronousEncryptorMock.hpp
    SynchronousFixedRandomDataGenerator.cpp
    SynchronousFixedRandomDataGenerator.hpp
    SynchronousFlashStub.cpp
    SynchronousFlashStub.hpp
    SynchronousRandomDataGeneratorMock.hpp
    SynchronousSerialCommunicationMock.cpp
    SynchronousSerialCommunicationMock.hpp
)

add_subdirectory(test)