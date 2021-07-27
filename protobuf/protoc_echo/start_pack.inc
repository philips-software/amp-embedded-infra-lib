macro(protocol_buffer_echo_cpp input)

    if(NOT CCOLA_COMPONENT_SKIP)
        ccola_make_absolute_from_source(absolute_input ${input})
        ccola_sources("${absolute_input}")
        ccola_set_source_group("Source Files" "${absolute_input}")
        ccola_get_filename_component_we(source_base ${input})

        get_filename_component(generated_dir_echo "${CMAKE_CURRENT_BINARY_DIR}/generated/echo" ABSOLUTE)
        ccola_create_directory_on_demand("${generated_dir_echo}")

        ccola_get_executable_path(protoc_echo_plugin_path protobuf.protoc_echo_plugin)
        ccola_get_executable_path(protobuf_libprotobuf_path protobuf.libprotobuf)
        get_filename_component(protobuf_libprotobuf_path "${protobuf_libprotobuf_path}" DIRECTORY)
        ccola_translate_generator_expression(extra_path "${protobuf_libprotobuf_path}")

        string(REPLACE "\"" "" old_path "$ENV{PATH}")

        set(protopath)
        foreach(dependency ${PROTOBUF_DEPENDENCIES})
            ccola_read_public_property("${dependency}" proto paths)
            foreach(path ${paths})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        foreach(dependency_path "${CMAKE_CURRENT_SOURCE_DIR}" "${CCOLA_PACK_DIR_protoc}/../../protobuf-3.3.0/src/google/protobuf" "${CCOLA_PACK_DIR_protoc}/../../../protobuf/echo_attributes")
            foreach(path ${dependency_path})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        if(CMAKE_HOST_WIN32)
            set(error_format "msvs")
        else()
            set(error_format "gcc")
        endif()

        if (CMAKE_HOST_WIN32)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc.exe)
        elseif (CMAKE_HOST_APPLE)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-osx-x86_64)
        elseif (CMAKE_HOST_UNIX)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-linux-x86_64)
        else ()
            message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME}")
        endif()

        add_custom_command(
            OUTPUT "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=${extra_path};\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=${protoc_echo_plugin_path} --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=${extra_path};\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            DEPENDS "${absolute_input}"
            DEPENDS "${protoc_echo_plugin_path}"
        )

        ccola_sources("${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp")
        ccola_set_source_group("Generated Files" "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp")
        ccola_inheritable_include_directories("${CMAKE_CURRENT_BINARY_DIR}")
        ccola_public_property(proto "${CMAKE_CURRENT_SOURCE_DIR}")

        ccola_dependencies(
            protobuf.echo
        )
    endif()

endmacro()

macro(protocol_buffer_echo_csharp input)

    if(NOT CCOLA_COMPONENT_SKIP)
        ccola_make_absolute_from_source(absolute_input ${input})
        ccola_sources("${absolute_input}")
        ccola_set_source_group("Source Files" "${absolute_input}")
        ccola_get_filename_component_we(source_base ${input})

        get_filename_component(generated_dir_echo "${CMAKE_CURRENT_BINARY_DIR}/generated" ABSOLUTE)
        ccola_create_directory_on_demand("${generated_dir_echo}")

        ccola_get_executable_path(protoc_echo_plugin_path protobuf.protoc_echo_plugin_csharp)
        ccola_get_executable_path(protobuf_libprotobuf_path protobuf.libprotobuf)
        get_filename_component(protobuf_libprotobuf_path "${protobuf_libprotobuf_path}" DIRECTORY)
        ccola_translate_generator_expression(extra_path "${protobuf_libprotobuf_path}")

        string(REPLACE "\"" "" old_path "$ENV{PATH}")

        set(protopath)
        foreach(dependency ${PROTOBUF_DEPENDENCIES})
            ccola_read_public_property("${dependency}" proto paths)
            foreach(path ${paths})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        foreach(dependency_path "${CMAKE_CURRENT_SOURCE_DIR}" "${CCOLA_PACK_DIR_protoc}/../../protobuf-3.3.0/src/google/protobuf" "${CCOLA_PACK_DIR_protoc}/../../../protobuf/echo_attributes")
            foreach(path ${dependency_path})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        if(CMAKE_HOST_WIN32)
            set(error_format "msvs")
        else()
            set(error_format "gcc")
        endif()

        if (CMAKE_HOST_WIN32)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc.exe)
        elseif (CMAKE_HOST_APPLE)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-osx-x86_64)
        elseif (CMAKE_HOST_UNIX)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-linux-x86_64)
        else ()
            message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME}")
        endif()

        add_custom_command(
            OUTPUT "${generated_dir_echo}/${source_base}.pb.cs"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=${extra_path};${old_path};\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-csharp-echo=${protoc_echo_plugin_path} --csharp-echo_out="${generated_dir_echo}" ${absolute_input}
            DEPENDS "${absolute_input}"
        )

        ccola_sources("${generated_dir_echo}/${source_base}.pb.cs")
        ccola_set_source_group("Generated Files" "${generated_dir_echo}/${source_base}.pb.cs")

    endif()

endmacro()

macro(protocol_buffer_echo_java input)

    if(NOT CCOLA_COMPONENT_SKIP)
        ccola_make_absolute_from_source(absolute_input ${input})
        ccola_sources("${absolute_input}")
        ccola_set_source_group("Source Files" "${absolute_input}")
        ccola_get_filename_component_we(source_base ${input})

        get_filename_component(java_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/com/philips/cococo/protobufEcho" ABSOLUTE)
        get_filename_component(generated_dir_echo "${java_dir}" ABSOLUTE)
        ccola_create_directory_on_demand("${generated_dir_echo}")

        ccola_get_executable_path(protoc_echo_plugin_path protobuf.protoc_echo_plugin_java)
        ccola_get_executable_path(protobuf_libprotobuf_path protobuf.libprotobuf)
        get_filename_component(protobuf_libprotobuf_path "${protobuf_libprotobuf_path}" DIRECTORY)
        ccola_translate_generator_expression(extra_path "${protobuf_libprotobuf_path}")

        string(REPLACE "\"" "" old_path "$ENV{PATH}")

        set(protopath)
        foreach(dependency ${PROTOBUF_DEPENDENCIES})
            ccola_read_public_property("${dependency}" proto paths)
            foreach(path ${paths})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        foreach(dependency_path "${CMAKE_CURRENT_SOURCE_DIR}" "${CCOLA_PACK_DIR_protoc}/../../protobuf-3.3.0/src/google/protobuf" "${CCOLA_PACK_DIR_protoc}/../../../protobuf/echo_attributes")
            foreach(path ${dependency_path})
                list(APPEND protopath --proto_path ${path})
            endforeach()
        endforeach()

        if(CMAKE_HOST_WIN32)
            set(error_format "msvs")
        else()
            set(error_format "gcc")
        endif()

        if (CMAKE_HOST_WIN32)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc.exe)
        elseif (CMAKE_HOST_APPLE)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-osx-x86_64)
        elseif (CMAKE_HOST_UNIX)
            set(protoc_compiler_binary ${CCOLA_PACK_DIR_protoc}/../bin/protoc-linux-x86_64)
        else ()
            message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME}")
        endif()

        add_custom_command(
            OUTPUT "${generated_dir_echo}/${source_base}Services.java"
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${CMAKE_COMMAND} -E env "\"PATH=${extra_path};${old_path};\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-java-echo=${protoc_echo_plugin_path} --java-echo_out="${java_dir}" ${absolute_input}
            DEPENDS "${absolute_input}"
        )

        ccola_sources("${generated_dir_echo}/${source_base}Services.java")
        ccola_set_source_group("Generated Files" "${generated_dir_echo}/${source_base}Services.java")

    endif()

endmacro()

macro(protocol_buffer_echo_all input)

    protocol_buffer_echo_cpp(${input})
    protocol_buffer_echo_csharp(${input})
    protocol_buffer_echo_java(${input})

endmacro()
