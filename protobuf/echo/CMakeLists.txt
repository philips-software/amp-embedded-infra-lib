ccola_component(echo static_library)

    ccola_sources(
        Echo.cpp
        Echo.hpp
    )

    ccola_dependencies(
        infra.stream
        infra.syntax
        services.network
    )

ccola_end_component()
