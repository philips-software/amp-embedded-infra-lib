add_library(protobuf.test_helper EXCLUDE_FROM_ALL STATIC)

target_link_libraries(protobuf.test_helper
    PUBLIC

    protobuf.echo
)

target_sources(protobuf.test_helper PRIVATE
    EchoSingleLoopback.cpp
    EchoSingleLoopback.hpp
)
