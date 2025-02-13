if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING AND NOT EMIL_FETCH_ECHO_COMPILERS)
    add_executable(protobuf.protoc_echo_plugin_csharp)
    emil_build_for(protobuf.protoc_echo_plugin_csharp BOOL EMIL_BUILD_ECHO_COMPILERS)
    emil_install(protobuf.protoc_echo_plugin_csharp EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_csharp PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_csharp PRIVATE
        Main.cpp
        ProtoCEchoPluginCSharp.cpp
        ProtoCEchoPluginCSharp.hpp
    )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Workaround for warning C4141: 'inline': used more than once
        # Since we set warning as error and protobuf is adding inline twice, the earning C4141 is raised.
        # Issue on protobuf side: https://github.com/protocolbuffers/protobuf/issues/17821
        set_target_properties(protobuf.protoc_echo_plugin_csharp PROPERTIES COMPILE_WARNING_AS_ERROR Off)
    endif()
endif()
