if (EMIL_HOST_BUILD)
    add_executable(services.echo_console ${EMIL_EXCLUDE_FROM_ALL})
    emil_install(services.echo_console DESTINATION bin)

    target_link_libraries(services.echo_console PUBLIC
        args
        hal.generic
        protobuf.protoc_echo_plugin_lib
        protobuf.echo
        services.network_instantiations
        services.util
    )

    if (EMIL_BUILD_WIN)
        target_link_libraries(services.echo_console PRIVATE hal.windows)
    endif()

    if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
        target_link_libraries(services.echo_console PRIVATE hal.unix)
    endif()

    target_sources(services.echo_console PRIVATE
        Console.cpp
        Console.hpp
        Main.cpp
    )

    add_executable(services.echo_test_tcp_server ${EMIL_EXCLUDE_FROM_ALL})
    emil_install(services.echo_test_tcp_server DESTINATION bin) 

    target_link_libraries(services.echo_test_tcp_server PUBLIC
        args
        hal.generic
        protobuf.protoc_echo_plugin_lib
        protobuf.echo
        services.network_instantiations
        services.util
        protobuf.meta_services
    )

    if (EMIL_BUILD_WIN)
        target_link_libraries(services.echo_test_tcp_server PRIVATE hal.windows)
    endif()

    if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
        target_link_libraries(services.echo_test_tcp_server PRIVATE hal.unix)
    endif()

    target_sources(services.echo_test_tcp_server PRIVATE
        MainTestServer.cpp
    )
endif()
