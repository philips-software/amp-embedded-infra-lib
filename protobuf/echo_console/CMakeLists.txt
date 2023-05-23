add_executable(protobuf.echo_console)
emil_build_for(protobuf.echo_console HOST All PREREQUISITE_BOOL EMIL_STANDALONE)
install(TARGETS protobuf.echo_console EXPORT emilProtobufTargets DESTINATION bin)

target_link_libraries(protobuf.echo_console PUBLIC
    args
    hal.generic
    protobuf.protoc_echo_plugin_lib
    protobuf.echo
    services.network_instantiations
    services.util
)

if (EMIL_BUILD_WIN)
    target_link_libraries(protobuf.echo_console PRIVATE hal.windows)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_link_libraries(protobuf.echo_console PRIVATE hal.unix)
endif()

target_sources(protobuf.echo_console PRIVATE
    Console.cpp
    Console.hpp
    Main.cpp
)
