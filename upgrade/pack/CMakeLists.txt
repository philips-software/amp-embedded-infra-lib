ccola_component(pack source)

    ccola_sources(
        UpgradePackHeader.hpp
        KeyDefinitions.hpp
    )

    ccola_dependencies(
        infra.util
    )

ccola_end_component()

ccola_component(pack_keys static_library)

    protocol_buffer_echo_cpp(UpgradeKeys.proto)

ccola_end_component()
