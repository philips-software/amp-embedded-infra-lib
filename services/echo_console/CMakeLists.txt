if (EMIL_HOST_BUILD AND EMIL_INCLUDE_MBEDTLS)
    add_executable(services.echo_console ${EMIL_EXCLUDE_FROM_ALL})
    emil_install(services.echo_console DESTINATION bin)

    target_compile_options(services.echo_console PRIVATE
        # Clang complains about "captured structured bindings are a C++20 extension", which was an oversight in the C++ standard.
        # Rather than setting the language to C++20, this is a small thing that can simply be ignored
        $<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-extensions>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wno-c++20-extensions>
    )

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
