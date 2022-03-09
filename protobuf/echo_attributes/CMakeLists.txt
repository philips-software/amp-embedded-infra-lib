if (TARGET_ARCH_X86)
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS protobuf.echo_attributes EXPORT emilProtobufTargets)

    target_compile_options(protobuf.echo_attributes PRIVATE
        $<$<BOOL:${TARGET_BUILD_WIN}>:
            $<$<CXX_COMPILER_ID:MSVC>:
                /wd4018
                /wd4251
                /wd4273
                /wd4800
            >
        >
    )

    set_target_properties(protobuf.echo_attributes PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(protobuf.echo_attributes PUBLIC
        protobuf.libprotoc
    )

    target_compile_definitions(protobuf.echo_attributes PRIVATE
        LIBPROTOC_EXPORTS
    )

    protocol_buffer_cpp(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)
    protocol_buffer_csharp(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)
    protocol_buffer_java(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)
endif()
