add_library(protobuf.test_doubles STATIC)
emil_build_for(protobuf.test_doubles BOOL BUILD_TESTING)

target_link_libraries(protobuf.test_doubles PUBLIC
    gmock
    protobuf.echo
)

target_sources(protobuf.test_doubles PRIVATE
    EchoSingleLoopback.cpp
    EchoSingleLoopback.hpp
    EchoMock.hpp
    ServiceStub.cpp
    ServiceStub.hpp
)
