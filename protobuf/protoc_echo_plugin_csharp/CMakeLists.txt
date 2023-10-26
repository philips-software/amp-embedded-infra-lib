if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING)
    add_executable(protobuf.protoc_echo_plugin_csharp)
    emil_build_for(protobuf.protoc_echo_plugin_csharp BOOL EMIL_BUILD_ECHO_COMPILERS)
    emil_install(protobuf.protoc_echo_plugin_csharp DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_csharp PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_csharp PRIVATE
        Main.cpp
        ProtoCEchoPluginCSharp.cpp
        ProtoCEchoPluginCSharp.hpp
    )
endif()
