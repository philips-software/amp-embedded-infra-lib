if (EMIL_FETCH_ECHO_COMPILERS)
    set(NOT_EMIL_FETCH_ECHO_COMPILERS false)
else()
    set(NOT_EMIL_FETCH_ECHO_COMPILERS true)
endif()

add_executable(protobuf.protoc_echo_plugin_test)
emil_build_for(protobuf.protoc_echo_plugin_test BOOL EMIL_BUILD_TESTS PREREQUISITE_BOOL NOT_EMIL_FETCH_ECHO_COMPILERS)
emil_add_test(protobuf.protoc_echo_plugin_test)

protocol_buffer_echo_cpp(protobuf.protoc_echo_plugin_test ../../echo/test/TestMessages.proto)

target_sources(protobuf.protoc_echo_plugin_test PRIVATE
    TestProtoCEchoPlugin.cpp
)

target_link_libraries(protobuf.protoc_echo_plugin_test PUBLIC
    gmock_main
    infra.event_test_helper
    infra.util_test_helper
    protobuf.protoc_echo_plugin_lib
    services.network_test_doubles
)
