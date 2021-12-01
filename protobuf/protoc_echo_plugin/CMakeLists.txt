set(PROTOC_ECHO_PLUGIN_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "protoc echo plugin binary directory")

if(TARGET_ARCH_X86)
    add_library(protobuf.protoc_echo_plugin_lib EXCLUDE_FROM_ALL STATIC)

    target_compile_options(protobuf.protoc_echo_plugin_lib
        PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4251
                /wd4273
                /wd4800
            >
        >
    )

    target_compile_definitions(protobuf.protoc_echo_plugin_lib
        PRIVATE
        LIBPROTOC_EXPORTS
    )

    target_link_libraries(protobuf.protoc_echo_plugin_lib
        PUBLIC

        protobuf.libprotobuf
        protobuf.libprotoc
        protobuf.echo_attributes
        infra.stream
    )

    target_include_directories(protobuf.protoc_echo_plugin_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/embeddedinfralib/protobuf/protoc_echo_plugin
    )

    target_sources(protobuf.protoc_echo_plugin_lib PRIVATE
        CppFormatter.cpp
        CppFormatter.hpp
        EchoObjects.cpp
        EchoObjects.hpp
        Main.cpp
        ProtoCEchoPlugin.cpp
        ProtoCEchoPlugin.hpp
    )

    add_executable(protobuf.protoc_echo_plugin)

    target_compile_options(protobuf.protoc_echo_plugin
        PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4251
                /wd4273
                /wd4800
            >
        >
    )

    target_link_libraries(protobuf.protoc_echo_plugin
        PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin PRIVATE
        Main.cpp
    )

endif()
