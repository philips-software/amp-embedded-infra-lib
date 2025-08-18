if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING AND NOT EMIL_FETCH_ECHO_COMPILERS)
    add_executable(protobuf.protoc_echo_plugin_csharp)
    emil_build_for(protobuf.protoc_echo_plugin_csharp BOOL EMIL_BUILD_ECHO_COMPILERS)
    emil_install(protobuf.protoc_echo_plugin_csharp EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_csharp PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    # Disable deprecation warnings for protobuf headers to avoid build errors
    target_compile_options(protobuf.protoc_echo_plugin_csharp PRIVATE 
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
    )

    target_sources(protobuf.protoc_echo_plugin_csharp PRIVATE
        Main.cpp
        ProtoCEchoPluginCSharp.cpp
        ProtoCEchoPluginCSharp.hpp
    )
endif()
