if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING AND NOT EMIL_FETCH_ECHO_COMPILERS)
    add_executable(protobuf.protoc_echo_plugin_java)
    emil_build_for(protobuf.protoc_echo_plugin_java BOOL EMIL_BUILD_ECHO_COMPILERS)
    emil_install(protobuf.protoc_echo_plugin_java EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_java PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_java PRIVATE
        Main.cpp
        ProtoCEchoPluginJava.cpp
        ProtoCEchoPluginJava.hpp
    )
endif()
