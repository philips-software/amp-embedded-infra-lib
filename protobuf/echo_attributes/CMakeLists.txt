if (EMIL_HOST_BUILD)
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS protobuf.echo_attributes EXPORT emilProtobufTargets)

    set_target_properties(protobuf.echo_attributes PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(protobuf.echo_attributes PUBLIC
        ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}libprotoc
    )

    target_compile_definitions(protobuf.echo_attributes PRIVATE
        LIBPROTOC_EXPORTS
    )

    protocol_buffer_all(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)
endif()
