ccola_component(tiny_aes128 static_library)

    ccola_inheritable_include_directories(.)

    ccola_sources(
        TinyAes.c
        TinyAes.h
    )

ccola_end_component()
