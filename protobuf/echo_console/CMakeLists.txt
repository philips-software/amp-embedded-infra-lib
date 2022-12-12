if (EMIL_HOST_BUILD)
    add_executable(protobuf.echo_console)
    install(TARGETS protobuf.echo_console EXPORT emilProtobufTargets)

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
        target_compile_definitions(protobuf.echo_console PRIVATE ECHO_CONSOLE_WITH_UART)
    endif()

    target_sources(protobuf.echo_console PRIVATE
        Console.cpp
        Console.hpp
        Main.cpp
    )

    emil_clangformat_target(protobuf.echo_console)
endif()
