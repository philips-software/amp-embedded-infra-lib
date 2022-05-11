add_executable(protobuf.protoc_echo_plugin_test)
emil_add_test(protobuf.protoc_echo_plugin_test)

protocol_buffer_echo_cpp(protobuf.protoc_echo_plugin_test TestMessages.proto)

target_sources(protobuf.protoc_echo_plugin_test PRIVATE
    TestCppFormatter.cpp
    TestProtoCEchoPlugin.cpp
)

target_link_libraries(protobuf.protoc_echo_plugin_test PUBLIC
    gmock_main
    infra.event_test_helper
    infra.util_test_helper
    protobuf.protoc_echo_plugin_lib
    services.network_test_doubles
)

if (TARGET_BUILD_WIN)
    add_custom_command(TARGET protobuf.protoc_echo_plugin_test PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:protobuf.protoc_echo_plugin_test> $<TARGET_FILE_DIR:protobuf.protoc_echo_plugin_test>
        COMMAND_EXPAND_LISTS
    )
endif()
