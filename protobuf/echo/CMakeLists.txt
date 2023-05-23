add_library(protobuf.echo ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.echo PUBLIC
    infra.syntax
    services.network
)

target_sources(protobuf.echo PRIVATE
    Echo.cpp
    Echo.hpp
    EchoInstantiation.hpp
    EchoOnConnection.cpp
    EchoOnConnection.hpp
    EchoOnMessageCommunication.cpp
    EchoOnMessageCommunication.hpp
    ServiceForwarder.cpp
    ServiceForwarder.hpp
    TracingEcho.cpp
    TracingEcho.hpp
    TracingEchoOnConnection.cpp
    TracingEchoOnConnection.hpp
)

add_subdirectory(test_doubles)

include(protocol_buffer_echo.cmake)

add_subdirectory(test)
