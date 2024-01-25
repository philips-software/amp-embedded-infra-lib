add_library(protobuf.meta_services ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_csharp(protobuf.meta_services ServiceDiscovery.proto)
protocol_buffer_java(protobuf.meta_services ServiceDiscovery.proto)
protocol_buffer_echo_all(protobuf.meta_services ServiceDiscovery.proto)

target_sources(protobuf.meta_services PRIVATE
    ServiceDiscoveryEcho.cpp
    ServiceDiscoveryEcho.hpp
)

add_subdirectory(test)

