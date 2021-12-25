if (TARGET_ARCH_X86)
    add_executable(protobuf.protoc_echo_plugin_java)
    add_executable(emil::protobuf.protoc_echo_plugin_java ALIAS protobuf.protoc_echo_plugin_java)
    install(TARGETS protobuf.protoc_echo_plugin_java EXPORT emilProtobufHostTargets)

    target_link_libraries(protobuf.protoc_echo_plugin_java PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_java PRIVATE
        Main.cpp
        ProtoCEchoPluginJava.cpp
        ProtoCEchoPluginJava.hpp
    )
endif()
