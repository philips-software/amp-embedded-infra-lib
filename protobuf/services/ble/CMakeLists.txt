ccola_component(services_ble static_library)

    protocol_buffer_csharp(Gap.proto)
    protocol_buffer_java(Gap.proto)
    protocol_buffer_echo_cpp(Gap.proto)

ccola_end_component()
