add_library(protobuf.test_helper ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.test_helper PUBLIC
    protobuf.echo
)

target_sources(protobuf.test_helper PRIVATE
    EchoSingleLoopback.cpp
    EchoSingleLoopback.hpp
)

emil_clangformat_target(protobuf.test_helper)
