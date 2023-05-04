if (EMIL_HOST_BUILD)
    add_executable(protobuf.protoc_echo_plugin_java)
    install(TARGETS protobuf.protoc_echo_plugin_java EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_java PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_java PRIVATE
        Main.cpp
        ProtoCEchoPluginJava.cpp
        ProtoCEchoPluginJava.hpp
    )
endif()
