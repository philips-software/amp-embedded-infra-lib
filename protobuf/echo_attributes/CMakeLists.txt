ccola_component(echo_attributes win: static_library other: skip)

    ccola_compile_options(/wd4018 /wd4251 /wd4273 /wd4800)

    ccola_definitions(LIBPROTOC_EXPORTS)

    protocol_buffer_cpp(EchoAttributes.proto)
    protocol_buffer_csharp(EchoAttributes.proto)
    protocol_buffer_java(EchoAttributes.proto)

    ccola_dependencies(
        libprotobuf
        libprotoc
    )

ccola_end_component()
