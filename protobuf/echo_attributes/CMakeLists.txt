if (EMIL_HOST_BUILD)
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} STATIC)

    set_target_properties(protobuf.echo_attributes PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(protobuf.echo_attributes PUBLIC
        protobuf::libprotoc
    )

    target_compile_definitions(protobuf.echo_attributes PRIVATE
        LIBPROTOC_EXPORTS
    )

    protocol_buffer_all(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)
else()
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

    set_target_properties(protobuf.echo_attributes PROPERTIES PROTOBUF_INCLUDES ${CMAKE_CURRENT_LIST_DIR})
endif()

install(TARGETS protobuf.echo_attributes EXPORT emilProtobufTargets)
