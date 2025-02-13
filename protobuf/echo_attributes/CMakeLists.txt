if (EMIL_HOST_BUILD)
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} STATIC)

    set_target_properties(protobuf.echo_attributes PROPERTIES POSITION_INDEPENDENT_CODE ON)

    target_link_libraries(protobuf.echo_attributes PUBLIC
        libprotoc
        libprotobuf
    )

    target_compile_definitions(protobuf.echo_attributes PRIVATE
        LIBPROTOC_EXPORTS
    )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Workaround for warning C4141: 'inline': used more than once
        # Since we set warning as error and protobuf is adding inline twice, the earning C4141 is raised.
        # Issue on protobuf side: https://github.com/protocolbuffers/protobuf/issues/17821
        set_target_properties(protobuf.echo_attributes PROPERTIES COMPILE_WARNING_AS_ERROR Off)
    endif()

    protocol_buffer_all(protobuf.echo_attributes ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto)

    install(FILES ${CMAKE_CURRENT_LIST_DIR}/EchoAttributes.proto TYPE INCLUDE)
else()
    add_library(protobuf.echo_attributes ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

    set_target_properties(protobuf.echo_attributes PROPERTIES PROTOBUF_INCLUDES ${CMAKE_CURRENT_LIST_DIR})
endif()
