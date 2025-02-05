if (EMIL_HOST_BUILD AND NOT EMIL_FETCH_ECHO_COMPILERS)
    add_library(protobuf.protoc_echo_plugin_lib ${EMIL_EXCLUDE_FROM_ALL} STATIC)

    target_compile_definitions(protobuf.protoc_echo_plugin_lib PRIVATE
        LIBPROTOC_EXPORTS
    )

    target_link_libraries(protobuf.protoc_echo_plugin_lib PUBLIC
        protobuf.echo_attributes
        infra.syntax
    )

    target_include_directories(protobuf.protoc_echo_plugin_lib PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    target_sources(protobuf.protoc_echo_plugin_lib PRIVATE
        EchoObjects.cpp
        EchoObjects.hpp
        ProtoCEchoPlugin.cpp
        ProtoCEchoPlugin.hpp
    )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # Workaround for warning C4141: 'inline': used more than once
        # Since we set warning as error and protobuf is adding inline twice, the earning C4141 is raised.
        # Issue on protobuf side: https://github.com/protocolbuffers/protobuf/issues/17821
        set_target_properties(protobuf.protoc_echo_plugin_lib PROPERTIES COMPILE_WARNING_AS_ERROR Off)
    endif()

    if (NOT CMAKE_CROSSCOMPILING)
        add_executable(protobuf.protoc_echo_plugin)
        emil_build_for(protobuf.protoc_echo_plugin BOOL EMIL_BUILD_ECHO_COMPILERS)
        emil_install(protobuf.protoc_echo_plugin EXPORT emilProtobufTargets DESTINATION bin)

        target_link_libraries(protobuf.protoc_echo_plugin PUBLIC
            protobuf.protoc_echo_plugin_lib
        )

        target_sources(protobuf.protoc_echo_plugin PRIVATE
            Main.cpp
        )

        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            # Workaround for warning C4141: 'inline': used more than once
            # Since we set warning as error and protobuf is adding inline twice, the earning C4141 is raised.
            # Issue on protobuf side: https://github.com/protocolbuffers/protobuf/issues/17821
            set_target_properties(protobuf.protoc_echo_plugin PROPERTIES COMPILE_WARNING_AS_ERROR Off)
        endif()
    endif()
endif()

add_subdirectory(test)
