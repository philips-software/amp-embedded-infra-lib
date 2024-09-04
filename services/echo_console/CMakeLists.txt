if (EMIL_HOST_BUILD AND EMIL_INCLUDE_MBEDTLS)
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

    target_sources(services.echo_console PRIVATE
        Console.cpp
        Console.hpp
        Main.cpp
    )
endif()
