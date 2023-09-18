add_library(protobuf.echo ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.echo PUBLIC
    infra.syntax
    services.network
)

target_sources(protobuf.echo PRIVATE
    BufferingStreamReader.cpp
    BufferingStreamReader.hpp
    BufferingStreamWriter.cpp
    BufferingStreamWriter.hpp
    Echo.cpp
    Echo.hpp
    Proto.hpp
    ProtoMessageReceiver.cpp
    ProtoMessageReceiver.hpp
    ProtoMessageSender.cpp
    ProtoMessageSender.hpp
    ServiceForwarder.cpp
    ServiceForwarder.hpp
    TracingEcho.cpp
    TracingEcho.hpp
)

add_subdirectory(test_doubles)

include(protocol_buffer_echo.cmake)

add_subdirectory(test)
