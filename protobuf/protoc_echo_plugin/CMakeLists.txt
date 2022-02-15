if (TARGET_ARCH_X86)
    add_library(protobuf.protoc_echo_plugin_lib EXCLUDE_FROM_ALL STATIC)

    target_compile_options(protobuf.protoc_echo_plugin_lib PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4251
                /wd4273
                /wd4800
            >
        >
    )

    target_compile_definitions(protobuf.protoc_echo_plugin_lib PRIVATE
        LIBPROTOC_EXPORTS
    )

    target_link_libraries(protobuf.protoc_echo_plugin_lib PUBLIC
        protobuf.libprotobuf
        protobuf.libprotoc
        protobuf.echo_attributes
    )

    target_include_directories(protobuf.protoc_echo_plugin_lib PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    target_sources(protobuf.protoc_echo_plugin_lib PRIVATE
        CppFormatter.cpp
        CppFormatter.hpp
        EchoObjects.cpp
        EchoObjects.hpp
        ProtoCEchoPlugin.cpp
        ProtoCEchoPlugin.hpp
    )

    add_executable(protobuf.protoc_echo_plugin)

    target_link_libraries(protobuf.protoc_echo_plugin PUBLIC
        protobuf.protoc_echo_plugin_lib
    )

    target_sources(protobuf.protoc_echo_plugin PRIVATE
        Main.cpp
    )

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()

    install(TARGETS
        protobuf.protoc_echo_plugin_lib
        protobuf.protoc_echo_plugin
        EXPORT emilProtobufTargets
    )
endif()
