ccola_component(security_key_generator x86: executable other: noncompile)    

    protocol_buffer_echo_cpp(UpgradeKeys.proto)

    ccola_sources(
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )

    ccola_dependencies(
        crypto.mbed_tls
        hal.generic
        pack_builder
        external.args
    )

ccola_end_component()
