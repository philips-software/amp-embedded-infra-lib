function(protocol_buffer_echo_setvars target input)

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
    list(APPEND protobuf_dependencies ${target} ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.libprotobuf ${EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE}protobuf.echo_attributes)

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

    if (CMAKE_HOST_WIN32)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc.exe PARENT_SCOPE)
    elseif (CMAKE_HOST_APPLE)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc-osx-x86_64 PARENT_SCOPE)
    elseif (CMAKE_HOST_UNIX)
        set(protoc_compiler_binary ${PROTOC_DIR}/bin/protoc-linux-x86_64 PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "No suitable proto compiler found for ${CMAKE_HOST_SYSTEM_NAME}")
    endif()

endfunction()

include(protocol_buffer_echo_cpp.cmake)
include(protocol_buffer_echo_csharp.cmake)
include(protocol_buffer_echo_java.cmake)

function(protocol_buffer_echo_all target input)

    protocol_buffer_echo_cpp(${target} ${input})
    protocol_buffer_echo_csharp(${target} ${input})
    protocol_buffer_echo_java(${target} ${input})

endfunction()