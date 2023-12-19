add_executable(protobuf.echo_test)
emil_build_for(protobuf.echo_test BOOL EMIL_BUILD_TESTS)
emil_add_test(protobuf.echo_test)

protocol_buffer_echo_cpp(protobuf.echo_test TestMessages.proto)

target_sources(protobuf.echo_test PRIVATE
    TestProtoMessageReceiver.cpp
    TestProtoMessageSender.cpp
    TestServiceForwarder.cpp
)

target_link_libraries(protobuf.echo_test PUBLIC
    gmock_main
    infra.event_test_helper
    infra.util_test_helper
    protobuf.echo
    protobuf.test_doubles
)
