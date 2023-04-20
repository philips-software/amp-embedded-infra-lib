add_library(protobuf.test_doubles ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.test_doubles PUBLIC
    protobuf.echo
)

target_sources(protobuf.test_doubles PRIVATE
    EchoSingleLoopback.cpp
    EchoSingleLoopback.hpp
    EchoMock.hpp
)
