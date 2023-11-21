if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING)
    add_executable(protobuf.protoc_echo_plugin_java)
    emil_build_for(protobuf.protoc_echo_plugin_java BOOL EMIL_STANDALONE)
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
