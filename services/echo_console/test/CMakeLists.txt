add_executable(services.echo_console_test)
emil_build_for(services.echo_console_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.echo_console_test)

target_sources(services.echo_console_test PRIVATE
    TestConsoleService.cpp
)

target_link_libraries(services.echo_console_test PUBLIC
    gmock_main
    services.console_service
    protobuf.protoc_echo_plugin_lib
    protobuf.echo
    services.network_instantiations
    services.util
    protobuf.meta_services
)
