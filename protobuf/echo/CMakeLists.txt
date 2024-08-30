add_library(protobuf.echo ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.echo PUBLIC
    infra.syntax
    services.network
)

if (EMIL_ENABLE_GLOBAL_TRACING)
    target_compile_definitions(protobuf.echo PUBLIC EMIL_ENABLE_GLOBAL_TRACING)
endif()

target_sources(protobuf.echo PRIVATE
    Echo.cpp
    Echo.hpp
    EchoErrorPolicy.cpp
    EchoErrorPolicy.hpp
    Proto.hpp
    ProtoMessageReceiver.cpp
    ProtoMessageReceiver.hpp
    ProtoMessageSender.cpp
    ProtoMessageSender.hpp
    Serialization.cpp
    Serialization.hpp
    ServiceForwarder.cpp
    ServiceForwarder.hpp
    TracingEcho.cpp
    TracingEcho.hpp
)

add_subdirectory(test_doubles)

include(protocol_buffer_echo.cmake)

add_subdirectory(test)
