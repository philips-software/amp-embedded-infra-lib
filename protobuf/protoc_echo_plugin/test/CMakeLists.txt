ccola_component(protoc_echo_plugin_test gmock_test)

    ccola_compile_options(
    win:
        /wd4018 /wd4251 /wd4273 /wd4800
    )

	protocol_buffer_csharp(TestMessages.proto)
	protocol_buffer_java(TestMessages.proto)
	protocol_buffer_echo_cpp(TestMessages.proto)

    ccola_sources(
        TestCppFormatter.cpp
        TestProtoCEchoPlugin.cpp
    )

    ccola_dependencies(
        protoc_echo_plugin_lib
        infra.event_test_helper
        infra.util_test_helper
        services.network_test_doubles
    )

ccola_end_component()
