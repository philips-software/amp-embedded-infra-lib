ccola_component(security_key_generator x86: executable other: noncompile)

    ccola_sources(
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )

    ccola_dependencies(
        crypto.mbed_tls
        hal.generic
        pack_keys
        pack_builder
        external.args
    )

ccola_end_component()
