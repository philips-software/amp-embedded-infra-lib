function(protocol_buffer_echo_cpp target input)

    protocol_buffer_echo_setvars(${target} ${input})

    cmake_path(SET generated_dir_echo "generated/echo")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
        MAIN_DEPENDENCY "${absolute_input}"
        DEPENDS ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin
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