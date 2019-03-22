ccola_component(security_key_generator win: executable other: noncompile)

    ccola_sources(
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )

    ccola_dependencies(
        crypto.mbed_tls
        pack_builder
    )

ccola_end_component()
