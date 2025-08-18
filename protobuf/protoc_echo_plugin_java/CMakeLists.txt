if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING AND NOT EMIL_FETCH_ECHO_COMPILERS)
    add_executable(protobuf.protoc_echo_plugin_java)
    emil_build_for(protobuf.protoc_echo_plugin_java BOOL EMIL_BUILD_ECHO_COMPILERS)
    emil_install(protobuf.protoc_echo_plugin_java EXPORT emilProtobufTargets DESTINATION bin)

    target_link_libraries(protobuf.protoc_echo_plugin_java PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    # Disable deprecation warnings for protobuf headers to avoid build errors
    target_compile_options(protobuf.protoc_echo_plugin_java PRIVATE 
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations>
        $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
    )

    target_sources(protobuf.protoc_echo_plugin_java PRIVATE
        Main.cpp
        ProtoCEchoPluginJava.cpp
        ProtoCEchoPluginJava.hpp
    )
endif()
