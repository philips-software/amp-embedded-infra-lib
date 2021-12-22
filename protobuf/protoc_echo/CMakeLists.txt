function(protocol_buffer_echo_cpp target input)
    if (CMAKE_CROSSCOMPILING)
        message(WARNING "TODO: build proto generator when cross-compiling")
        # https://stackoverflow.com/questions/42506804/launch-host-and-target-crosscompiling-builds-inside-cmake
        # https://stackoverflow.com/questions/36173840/how-to-instruct-cmake-to-use-the-build-architecture-compiler
    else ()
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

        if(CMAKE_HOST_WIN32)    # TODO chage to compiler-specific condition
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
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            DEPENDS "${absolute_input}"
            DEPENDS protobuf.protoc_echo_plugin
        )

        target_sources(${target}
            PRIVATE
            ${generated_files}
        )
    
        target_include_directories(${target}
            PUBLIC
            ${CMAKE_CURRENT_BINARY_DIR}
        )
    
        target_link_libraries(${target}
            PUBLIC
            protobuf.echo
        )
    endif()
endfunction()
