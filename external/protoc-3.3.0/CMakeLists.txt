set(PROTOC_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "protoc-3.3.0 directory")

macro(protocol_buffer_cpp target input)

    ccola_make_absolute_from_source(absolute_input ${input})
    ccola_get_filename_component_we(source_base ${input})
    get_filename_component(absolute_directory_input ${absolute_input} DIRECTORY)

    get_filename_component(generated_dir_cpp "${CMAKE_CURRENT_BINARY_DIR}/generated/proto_cpp" ABSOLUTE)

    set(protopath)
    foreach(dependency ${PROTOBUF_DEPENDENCIES})
        emil_read_public_property("${dependency}" proto paths)
        foreach(path ${paths})
            list(APPEND protopath --proto_path ${path})
        endforeach()
    endforeach()

    foreach(dependency_path "${absolute_directory_input}" "${PROTOBUF_DIR}/src/google/protobuf" "${PROTOC_DIR}/../../protobuf/echo_attributes")
        foreach(path ${dependency_path})
            list(APPEND protopath --proto_path=${path})
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

    set(generated_files "${generated_dir_cpp}/${source_base}.pb.cc" "${generated_dir_cpp}/${source_base}.pb.h")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_cpp}
        COMMAND ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --cpp_out="${generated_dir_cpp}" ${absolute_input}
        DEPENDS "${absolute_input}"
    )

    target_sources(${target} PRIVATE ${generated_files})
    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${generated_dir_cpp})
    target_link_libraries(${target} PUBLIC protobuf.libprotobuf)

endmacro()


macro(protocol_buffer_dependencies)

    set(dependencies "${ARGV}")
    emil_parse_selection_list(dependencies)

    foreach(item ${dependencies})
        emil_make_full_component_name(item)
        emil_dependencies(${item})
        set(PROTOBUF_DEPENDENCIES ${PROTOBUF_DEPENDENCIES} ${item})
    endforeach()

endmacro()
