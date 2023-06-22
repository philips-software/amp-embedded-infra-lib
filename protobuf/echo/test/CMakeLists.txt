add_executable(protobuf.echo_test)
emil_build_for(protobuf.echo_test BOOL EMIL_BUILD_TESTS)
emil_add_test(protobuf.echo_test)

target_sources(protobuf.echo_test PRIVATE
    TestServiceForwarder.cpp
)

target_link_libraries(protobuf.echo_test PUBLIC
    gmock_main
    infra.event_test_helper
    infra.util_test_helper
    protobuf.echo
)
