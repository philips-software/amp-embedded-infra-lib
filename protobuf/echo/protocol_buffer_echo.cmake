function(protocol_buffer_echo_generator target input)
    if (CMAKE_CROSSCOMPILING)
        find_package(emil COMPONENTS Infra Hal Crypto Services Protobuf REQUIRED)
    endif()

    cmake_path(ABSOLUTE_PATH input NORMALIZE OUTPUT_VARIABLE absolute_input)
    cmake_path(GET absolute_input STEM source_base)
    cmake_path(GET absolute_input PARENT_PATH absolute_directory_input)

    set_target_properties(${target} PROPERTIES
        PROTOBUF_INCLUDES "${absolute_directory_input}"
        EXPORT_PROPERTIES PROTOBUF_INCLUDES
    )

    # Build a list of protobuf dependencies set on current target.
    # For each of these targets, get the PROTOBUF_INCLUDES property
    # for a list of additional paths to .proto files.
    get_target_property(protobuf_dependencies ${target} PROTOBUF_DEPENDENCIES)
    if (NOT protobuf_dependencies)
        set(protobuf_dependencies)
    endif()

    # Include defaults and self. This assumes set_target_properties on self
    # is done prior to the loop.
    list(APPEND protobuf_dependencies ${target} ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.echo_attributes)

    set(protopath)
    foreach(dependency ${protobuf_dependencies})
        get_target_property(path ${dependency} PROTOBUF_INCLUDES)
        if (NOT ${path} IN_LIST protopath)
            list(APPEND protopath --proto_path ${path})
        endif()
    endforeach()

    # Apply all required variables to PARENT_SCOPE  in one place from here onwards
    set(absolute_input ${absolute_input} PARENT_SCOPE)
    set(source_base ${source_base} PARENT_SCOPE)

    set(protopath ${protopath} PARENT_SCOPE)

    if (MSVC)
        set(error_format "msvs" PARENT_SCOPE)
    else()
        set(error_format "gcc" PARENT_SCOPE)
    endif()

    emil_fetch_protocol_buffer_compiler()
endfunction()

function(protocol_buffer_echo_cpp target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET generated_dir_echo "generated/echo")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}libprotobuf>;\"" ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}libprotobuf>;\"" ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
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

function(protocol_buffer_echo_csharp target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET generated_dir_echo "generated")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cs")

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}libprotobuf>;\"" ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-csharp-echo=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin_csharp> --csharp-echo_out="${generated_dir_echo}" ${absolute_input}
        BYPRODUCTS ${generated_files}
    )
endfunction()

function(protocol_buffer_echo_java target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET java_dir "generated/com/philips/cococo/protobufEcho")
    cmake_path(SET generated_dir_echo "${java_dir}")

    cmake_path(ABSOLUTE_PATH java_dir BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE java_dir)
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}Services.java")

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}libprotobuf>;\"" ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-java-echo=$<TARGET_FILE:${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.protoc_echo_plugin_java> --java-echo_out="${java_dir}" ${absolute_input}
        BYPRODUCTS ${generated_files}
    )
endfunction()

function(protocol_buffer_echo_all target input)
    protocol_buffer_echo_cpp(${target} ${input})
    protocol_buffer_echo_csharp(${target} ${input})
    protocol_buffer_echo_java(${target} ${input})
endfunction()
