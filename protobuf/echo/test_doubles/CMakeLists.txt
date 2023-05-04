add_library(protobuf.test_doubles STATIC)
emil_build_for(protobuf.test_doubles PREREQUISITE_BOOL BUILD_TESTING)

target_link_libraries(protobuf.test_doubles PUBLIC
    protobuf.echo
)

target_sources(protobuf.test_doubles PRIVATE
    EchoSingleLoopback.cpp
    EchoSingleLoopback.hpp
    EchoMock.hpp
)
