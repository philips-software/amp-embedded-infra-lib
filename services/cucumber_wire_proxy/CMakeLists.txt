if (EMIL_HOST_BUILD)
    add_executable(services.cucumber_wire_proxy)
    install(TARGETS services.cucumber_wire_proxy EXPORT emilServicesTargets)

    add_version_header_dependency(services.cucumber_wire_proxy generated.version_header)

    target_link_libraries(services.cucumber_wire_proxy PUBLIC
        args
        hal.generic
        services.cucumber
        services.network_instantiations
    )

    if (EMIL_BUILD_WIN)
        target_compile_definitions(services.cucumber_wire_proxy PRIVATE EMIL_BUILD_WIN)
    endif()

    if (EMIL_BUILD_UNIX)
        target_link_options(services.cucumber_wire_proxy PRIVATE
            "-rdynamic"
        )
    endif()

    target_sources(services.cucumber_wire_proxy PRIVATE
        Main.cpp
    )
endif()
