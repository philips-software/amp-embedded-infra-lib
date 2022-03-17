
function(protocol_buffer_echo_csharp target input)

    protocol_buffer_echo_setvars(${target} ${input})

    cmake_path(SET generated_dir_echo "generated")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cs")

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-csharp-echo=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin_csharp> --csharp-echo_out="${generated_dir_echo}" ${absolute_input}
        
        BYPRODUCTS ${generated_files}
    )

endfunction()
