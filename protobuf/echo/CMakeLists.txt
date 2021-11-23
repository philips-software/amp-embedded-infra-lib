ccola_component(echo static_library)

    ccola_sources(
        Echo.cpp
        Echo.hpp
        EchoInstantiation.hpp
    )

    ccola_dependencies(
        infra.stream
        infra.syntax
        services.network
    )

ccola_end_component()

ccola_subdirectories(
    test_helper
)
