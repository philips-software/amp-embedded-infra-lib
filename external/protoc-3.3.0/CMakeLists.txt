set(PROTOC_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "protoc-3.3.0 directory")

function(protocol_buffer_generator)

    set(singleArgs TARGET INPUT GENERATOR PATH_INFIX)
    set(multiArgs GENERATED_POSTFIXES)
    cmake_parse_arguments(PARSE_ARGV 0 PROTOGEN "" "${singleArgs}" "${multiArgs}")

    cmake_path(ABSOLUTE_PATH PROTOGEN_INPUT NORMALIZE OUTPUT_VARIABLE absolute_input)
    cmake_path(GET absolute_input STEM input_stem)
    cmake_path(GET absolute_input PARENT_PATH input_parent_path)
    cmake_path(SET output "generated")
    cmake_path(ABSOLUTE_PATH output BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir)

    if (PROTOGEN_PATH_INFIX)
        cmake_path(SET path_infix "${PROTOGEN_PATH_INFIX}")
    else()
        set(path_infix)
    endif()

    set(protopath)
    get_target_property(protobuf_dependencies ${PROTOGEN_TARGET} PROTOBUF_DEPENDENCIES)
    if (protobuf_dependencies)
        foreach(dependency ${protobuf_dependencies})
            get_target_property(path ${dependency} SOURCE_DIR)
            list(APPEND protopath --proto_path ${path})
        endforeach()
    endif()

    get_target_property(protobuf_dir emil::protobuf.libprotobuf SOURCE_DIR)

    foreach(dependency_path "${input_parent_path}" "${protobuf_dir}/src/google/protobuf" "${PROTOC_DIR}/../../protobuf/echo_attributes")
        foreach(path ${dependency_path})
            list(APPEND protopath --proto_path=${path})
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

    set(generated_files)
    foreach(postfix_item "${PROTOGEN_GENERATED_POSTFIXES}")
        foreach(postfix ${postfix_item})
            list(APPEND generated_files "${generated_dir}/${path_infix}${input_stem}${postfix}")
        endforeach()
    endforeach()

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir}
        COMMAND ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --${PROTOGEN_GENERATOR}_out="${generated_dir}" ${absolute_input}
        MAIN_DEPENDENCY "${absolute_input}"
    )

    target_sources(${PROTOGEN_TARGET} PRIVATE
        ${generated_files}
    )

    set_source_files_properties(${generated_files} PROPERTIES FOLDER "Generated")

    target_include_directories(${PROTOGEN_TARGET} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${generated_dir}>"
    )

endfunction()

function(protocol_buffer_cpp target input)

    protocol_buffer_generator(TARGET ${target} INPUT ${input} GENERATOR cpp GENERATED_POSTFIXES .pb.h .pb.cc)

    target_link_libraries(${target} PUBLIC
        protobuf.libprotobuf
    )

endfunction()

function(protocol_buffer_csharp target input)

    protocol_buffer_generator(TARGET ${target} INPUT ${input} GENERATOR csharp GENERATED_POSTFIXES .cs)

endfunction()

function(protocol_buffer_java target input)

    protocol_buffer_generator(TARGET ${target} INPUT ${input} GENERATOR java PATH_INFIX "/com/philips/emil/ProtobufEcho/" GENERATED_POSTFIXES Proto.java)

endfunction()

function(protocol_buffer_dependencies target)

    set_target_properties(${target} PROPERTIES PROTOBUF_DEPENDENCIES "${ARGN}")

endfunction()
