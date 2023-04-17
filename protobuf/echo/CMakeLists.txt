add_library(protobuf.echo ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(protobuf.echo PUBLIC
    infra.syntax
    services.network
)

target_sources(protobuf.echo PRIVATE
    Echo.cpp
    Echo.hpp
    EchoInstantiation.hpp
    TracingEcho.cpp
    TracingEcho.hpp
)

if (BUILD_TESTING)
    add_subdirectory(test_helper)
endif()

include(protocol_buffer_echo.cmake)
