function(emil_fetch_protocol_buffer_compiler)
    set(protobuf_version "21.12")

    if (CMAKE_HOST_WIN32)
        FetchContent_Declare(protoc
            URL https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_version}/protoc-${protobuf_version}-win32.zip
        )
    elseif (CMAKE_HOST_APPLE)
        FetchContent_Declare(protoc
            URL https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_version}/protoc-${protobuf_version}-osx-universal_binary.zip
        )
    elseif (CMAKE_HOST_UNIX)
        string(REPLACE "aarch64" "aarch_64" protoc_arch ${CMAKE_HOST_SYSTEM_PROCESSOR})
        FetchContent_Declare(protoc
            URL https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_version}/protoc-${protobuf_version}-linux-${protoc_arch}.zip
        )
    else()
        message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME} (${CMAKE_HOST_SYSTEM_PROCESSOR})")
    endif()

    FetchContent_MakeAvailable(protoc)

    if (CMAKE_HOST_WIN32)
        set(EMIL_PROTOC_COMPILER_BINARY "${protoc_SOURCE_DIR}/bin/protoc.exe" CACHE INTERNAL "")
    else()
        set(EMIL_PROTOC_COMPILER_BINARY "${protoc_SOURCE_DIR}/bin/protoc" CACHE INTERNAL "")
    endif()
endfunction()

function(protocol_buffer_generator)
    if (CMAKE_CROSSCOMPILING)
        find_package(emil COMPONENTS Infra Hal Crypto Services Protobuf REQUIRED)
    endif()

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

    set_target_properties(${PROTOGEN_TARGET} PROPERTIES
        PROTOBUF_INCLUDES "${input_parent_path}"
        EXPORT_PROPERTIES PROTOBUF_INCLUDES
    )

    # Build a list of protobuf dependencies set on current target.
    # For each of these targets, get the PROTOBUF_INCLUDES property
    # for a list of additional paths to .proto files.
    get_target_property(protobuf_dependencies ${PROTOGEN_TARGET} PROTOBUF_DEPENDENCIES)
    if (NOT protobuf_dependencies)
        set(protobuf_dependencies)
    endif()

    # Include defaults and self. This assumes set_target_properties on self
    # is done prior to the loop.
    list(APPEND protobuf_dependencies ${PROTOGEN_TARGET} ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.echo_attributes)

    set(protopath)
    foreach(dependency ${protobuf_dependencies})
        get_target_property(path ${dependency} PROTOBUF_INCLUDES)
        if (NOT ${path} IN_LIST protopath)
            list(APPEND protopath --proto_path ${path})
        endif()
    endforeach()

    emil_fetch_protocol_buffer_compiler()

    set(generated_files)
    foreach(postfix_item "${PROTOGEN_GENERATED_POSTFIXES}")
        foreach(postfix ${postfix_item})
            list(APPEND generated_files "${generated_dir}/${path_infix}${input_stem}${postfix}")
        endforeach()
    endforeach()

    if (MSVC)
        set(error_format "msvs")
    else()
        set(error_format "gcc")
    endif()

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir}
        COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --${PROTOGEN_GENERATOR}_out="${generated_dir}" ${absolute_input}
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
        protobuf::libprotobuf
    )
endfunction()

function(protocol_buffer_csharp target input)
    protocol_buffer_generator(TARGET ${target} INPUT ${input} GENERATOR csharp GENERATED_POSTFIXES .cs)
endfunction()

function(protocol_buffer_java target input)
    protocol_buffer_generator(TARGET ${target} INPUT ${input} GENERATOR java PATH_INFIX "/com/philips/emil/ProtobufEcho/" GENERATED_POSTFIXES Proto.java)
endfunction()

function(protocol_buffer_all target input)
    protocol_buffer_cpp(${target} ${input})
    protocol_buffer_csharp(${target} ${input})
    protocol_buffer_java(${target} ${input})
endfunction()

function(protocol_buffer_dependencies target)
    set_target_properties(${target} PROPERTIES PROTOBUF_DEPENDENCIES "${ARGN}")
    target_link_libraries(${target} PUBLIC "${ARGN}")
endfunction()
