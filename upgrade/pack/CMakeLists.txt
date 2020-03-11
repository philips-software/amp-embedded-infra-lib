ccola_component(pack static_library)

    protocol_buffer_echo_cpp(UpgradeKeys.proto)

    ccola_sources(
        UpgradePackHeader.hpp
        KeyDefinitions.hpp
    )

    ccola_dependencies(
        infra.util
    )

ccola_end_component()
