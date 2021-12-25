function(protocol_buffer_echo_cpp target input)
    if (CMAKE_CROSSCOMPILING)
        find_package(emil 2.0.0 COMPONENTS ProtobufHost)
    endif()

    ccola_make_absolute_from_source(absolute_input ${input})
    ccola_get_filename_component_we(source_base ${input})
    get_filename_component(absolute_directory_input ${absolute_input} DIRECTORY)

    get_filename_component(generated_dir_echo "${CMAKE_CURRENT_BINARY_DIR}/generated/echo" ABSOLUTE)

    set(protopath)
    # foreach(dependency ${PROTOBUF_DEPENDENCIES})
    #     emil_read_public_property("${dependency}" proto paths)
    #     foreach(path ${paths})
    #         list(APPEND protopath --proto_path ${path})
    #     endforeach()
    # endforeach()

    foreach(dependency_path "${absolute_directory_input}" "${PROTOBUF_DIR}/src/google/protobuf" "${ECHO_ATTRIBUTES_DIR}")
        foreach(path ${dependency_path})
            list(APPEND protopath "--proto_path=${path}")
        endforeach()
    endforeach()

    if (MSVC)
        set(error_format "msvs")
    else()
        set(error_format "gcc")
    endif()

    if (CMAKE_HOST_WIN32)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc.exe)
    elseif (CMAKE_HOST_APPLE)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc-osx-x86_64)
    elseif (CMAKE_HOST_UNIX)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc-linux-x86_64)
    else ()
        message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME}")
    endif()

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:emil::protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:emil::protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:emil::protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
        DEPENDS "${absolute_input}"
        DEPENDS emil::protobuf.protoc_echo_plugin
    )

    target_sources(${target} PRIVATE
        ${generated_files}
    )
    
    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )
    
    target_link_libraries(${target} PUBLIC
        protobuf.echo
    )
endfunction()
