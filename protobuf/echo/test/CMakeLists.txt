add_executable(protobuf.echo_test)
emil_add_test(protobuf.echo_test)

target_sources(protobuf.echo_test PRIVATE
    ServiceStub.cpp
    ServiceStub.hpp
    TestEchoOnConnection.cpp
    TestEchoOnMessageCommunication.cpp
    TestServiceForwarder.cpp
)

target_link_libraries(protobuf.echo_test PUBLIC
    gmock_main
    infra.event_test_helper
    infra.util_test_helper
    protobuf.echo
    services.network_test_doubles
)
