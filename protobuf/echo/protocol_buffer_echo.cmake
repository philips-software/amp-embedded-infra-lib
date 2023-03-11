function(emil_fetch_echo_plugins)
    set(emil_version "3.0.0") # x-release-please-version

    if (CMAKE_HOST_WIN32)
        FetchContent_Declare(echoplugins
            URL https://github.com/philips-software/amp-embedded-infra-lib/releases/download/v${emil_version}/emil-${emil_version}-win64.zip
        )
    elseif (CMAKE_HOST_APPLE)
        FetchContent_Declare(echoplugins
            URL https://github.com/philips-software/amp-embedded-infra-lib/releases/download/v${emil_version}/emil-${emil_version}-Darwin.zip
        )
    elseif (CMAKE_HOST_UNIX)
        FetchContent_Declare(echoplugins
            URL https://github.com/philips-software/amp-embedded-infra-lib/releases/download/v${emil_version}/emil-${emil_version}-Linux.zip
        )
    else()
        message(FATAL_ERROR "No suitable echo plugin found for ${CMAKE_HOST_SYSTEM_NAME} (${CMAKE_HOST_SYSTEM_PROCESSOR})")
    endif()

    FetchContent_MakeAvailable(echoplugins)

    if (CMAKE_HOST_WIN32)
        set(host_executable_postfix ".exe")
    endif()

    set(EMIL_ECHO_CPP_COMPILER_BINARY "${echoplugins_SOURCE_DIR}/bin/protobuf.protoc_echo_plugin${host_executable_postfix}" CACHE INTERNAL "")
    set(EMIL_ECHO_CSHARP_COMPILER_BINARY "${echoplugins_SOURCE_DIR}/bin/protobuf.protoc_echo_plugin_csharp${host_executable_postfix}" CACHE INTERNAL "")
    set(EMIL_ECHO_JAVA_COMPILER_BINARY "${echoplugins_SOURCE_DIR}/bin/protobuf.protoc_echo_plugin_java${host_executable_postfix}" CACHE INTERNAL "")
endfunction()

function(protocol_buffer_echo_generator target input)
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
    list(APPEND protobuf_dependencies ${target} protobuf.echo_attributes)

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

    if (NOT EMIL_HOST_BUILD)
        emil_fetch_echo_plugins()
    endif()
endfunction()

function(protocol_buffer_echo_cpp target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET generated_dir_echo "generated/echo")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/Tracing${source_base}.pb.cpp" "${generated_dir_echo}/Tracing${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb")

    if (EMIL_HOST_BUILD)
        add_custom_command(
            OUTPUT ${generated_files}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            MAIN_DEPENDENCY "${absolute_input}"
            DEPENDS protobuf.protoc_echo_plugin
        )
    else()
        add_custom_command(
            OUTPUT ${generated_files}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=${EMIL_ECHO_CPP_COMPILER_BINARY} --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            MAIN_DEPENDENCY "${absolute_input}"
        )
    endif()

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

    if (EMIL_HOST_BUILD)
        set(compiler_binary $<TARGET_FILE:protobuf.protoc_echo_plugin_csharp>)
    else()
        set(compiler_binary ${EMIL_ECHO_CSHARP_COMPILER_BINARY})
    endif()

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-csharp-echo=${compiler_binary} --csharp-echo_out="${generated_dir_echo}" ${absolute_input}
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

    if (EMIL_HOST_BUILD)
        set(compiler_binary $<TARGET_FILE:protobuf.protoc_echo_plugin_java>)
    else()
        set(compiler_binary ${EMIL_ECHO_JAVA_COMPILER_BINARY})
    endif()

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-java-echo=${compiler_binary} --java-echo_out="${java_dir}" ${absolute_input}
        BYPRODUCTS ${generated_files}
    )
endfunction()

function(protocol_buffer_echo_all target input)
    protocol_buffer_echo_cpp(${target} ${input})
    protocol_buffer_echo_csharp(${target} ${input})
    protocol_buffer_echo_java(${target} ${input})
endfunction()
