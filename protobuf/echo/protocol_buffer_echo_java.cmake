
function(protocol_buffer_echo_java target input)

    protocol_buffer_echo_setvars(${target} ${input})

    cmake_path(SET java_dir "generated/com/philips/cococo/protobufEcho")
    cmake_path(SET generated_dir_echo "${java_dir}")

    cmake_path(ABSOLUTE_PATH java_dir BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE java_dir)
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}Services.java")

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-java-echo=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin_java> --java-echo_out="${java_dir}" ${absolute_input}
        
        BYPRODUCTS ${generated_files}
    )

endfunction()
