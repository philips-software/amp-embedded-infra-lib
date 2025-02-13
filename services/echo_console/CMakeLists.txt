if (EMIL_HOST_BUILD AND EMIL_INCLUDE_MBEDTLS)
    add_executable(services.echo_console ${EMIL_EXCLUDE_FROM_ALL})
    emil_install(services.echo_console DESTINATION bin)

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Workaround for warning C4141: 'inline': used more than once
        # Since we set warning as error and protobuf is adding inline twice, the earning C4141 is raised.
        # Issue on protobuf side: https://github.com/protocolbuffers/protobuf/issues/17821
        set_target_properties(services.echo_console PROPERTIES COMPILE_WARNING_AS_ERROR Off)
    endif()

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
