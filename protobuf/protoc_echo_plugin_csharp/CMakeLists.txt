if (TARGET_ARCH_X86)
    add_executable(protobuf.protoc_echo_plugin_csharp)
    add_executable(emil::protobuf.protoc_echo_plugin_csharp ALIAS protobuf.protoc_echo_plugin_csharp)
    install(TARGETS protobuf.protoc_echo_plugin_csharp EXPORT emilProtobufHostTargets)

    target_compile_options(protobuf.protoc_echo_plugin_csharp PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4273
            >
        >
    )

    target_link_libraries(protobuf.protoc_echo_plugin_csharp PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin_csharp PRIVATE
        Main.cpp
        ProtoCEchoPluginCSharp.cpp
        ProtoCEchoPluginCSharp.hpp
    )
endif()
