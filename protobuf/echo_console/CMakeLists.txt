if (EMIL_BUILD_WIN)
    add_executable(protobuf.echo_console)

    target_link_libraries(protobuf.echo_console PUBLIC
        args
        protobuf.protoc_echo_plugin_lib
        protobuf.echo
        hal.generic
        hal.windows
        services.network_win
        services.util
    )

    target_sources(protobuf.echo_console PRIVATE
        Console.cpp
        Console.hpp
        Main.cpp
    )

    emil_clangformat_target(protobuf.echo_console)
endif()
