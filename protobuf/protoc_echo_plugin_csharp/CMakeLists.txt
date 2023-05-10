if (EMIL_HOST_BUILD)
    add_executable(protobuf.protoc_echo_plugin_csharp)
    emil_build_for(protobuf.protoc_echo_plugin_csharp BOOL EMIL_STANDALONE)
    install(TARGETS protobuf.protoc_echo_plugin_csharp EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_csharp PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_csharp PRIVATE
        Main.cpp
        ProtoCEchoPluginCSharp.cpp
        ProtoCEchoPluginCSharp.hpp
    )
endif()
