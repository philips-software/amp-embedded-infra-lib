ccola_component(security_key_generator x86: executable other: noncompile)    

    ccola_sources(
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )

    ccola_add_version_header_dependency()

    ccola_dependencies(
        crypto.mbed_tls
        hal.generic
        pack_builder
        pack
        external.args
    )

ccola_end_component()
