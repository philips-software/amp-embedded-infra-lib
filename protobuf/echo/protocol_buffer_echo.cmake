function(emil_fetch_echo_plugins)
    # This function will first try to use `find_package` to import
    # echo plug-in executables. If there is an installed version
    # of emil present those echo plug-ins will be used.
    # Otherwise the latest emil release will be downloaded and those
    # echo plug-ins will be used instead.
    #
    # (See: https://cmake.org/cmake/help/latest/module/FetchContent.html#commands)

    if (EMIL_HOST_BUILD AND NOT CMAKE_CROSSCOMPILING AND NOT EMIL_FETCH_ECHO_COMPILERS)
        # In a host build where we are not cross-compiling we use the built echo plug-ins
        return()
    endif()

    FetchContent_GetProperties(echoplugin)
    if (echoplugin_POPULATED)
        return()
    endif()

    set(emil_version "6.1.0") # x-release-please-version

    if (CMAKE_HOST_WIN32)
        set(os_postfix "win64")
        set(host_executable_postfix ".exe")
    elseif (CMAKE_HOST_APPLE)
        set(os_postfix "Darwin")
    elseif (CMAKE_HOST_UNIX)
        set(os_postfix "Linux")
    else()
        message(FATAL_ERROR "No suitable echo plug-in found for ${CMAKE_HOST_SYSTEM_NAME} (${CMAKE_HOST_SYSTEM_PROCESSOR})")
    endif()

    FetchContent_Declare(echoplugin
        URL https://github.com/philips-software/amp-embedded-infra-lib/releases/download/v${emil_version}/emil-${emil_version}-${os_postfix}.zip
        FIND_PACKAGE_ARGS NAMES emil GLOBAL
    )
    FetchContent_MakeAvailable(echoplugin)

    if (NOT ${echoplugin_FOUND})
        foreach(language IN ITEMS "" "_csharp" "_java")
            if (NOT TARGET protobuf.protoc_echo_plugin${language})
                add_executable(protobuf.protoc_echo_plugin${language} IMPORTED GLOBAL)
                set_target_properties(protobuf.protoc_echo_plugin${language} PROPERTIES
                    IMPORTED_LOCATION "${echoplugin_SOURCE_DIR}/bin/protobuf.protoc_echo_plugin${language}${host_executable_postfix}"
                )
            endif()
        endforeach()
    else()
        message(STATUS "Using echo plug-ins from installed location")
    endif()
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

    # Apply all required variables to PARENT_SCOPE in one place from here onwards
    set(absolute_input ${absolute_input} PARENT_SCOPE)
    set(source_base ${source_base} PARENT_SCOPE)

    set(protopath ${protopath} PARENT_SCOPE)

    if (MSVC)
        set(error_format "msvs" PARENT_SCOPE)
    else()
        set(error_format "gcc" PARENT_SCOPE)
    endif()

    emil_fetch_protocol_buffer_compiler()
    emil_fetch_echo_plugins()
endfunction()

function(protocol_buffer_echo_cpp target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET generated_dir_echo "generated/echo")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)
    cmake_path(GET absolute_input FILENAME input_name)

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/Tracing${source_base}.pb.cpp" "${generated_dir_echo}/Tracing${source_base}.pb.hpp")
    set(dependency_file "${CMAKE_CURRENT_BINARY_DIR}/${input_name}.dep")

    # The generated dependency files lists these files in this order. Apparently, it is important that the OUTPUT lists these file in the exact same order.
    list(SORT generated_files)
    list(APPEND generated_files "${generated_dir_echo}/${source_base}.pb")

    if (CMAKE_GENERATOR STREQUAL Ninja)
        # Not all generators support the DEPFILE argument
        add_custom_command(
            OUTPUT ${generated_files}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:protobuf.protoc_echo_plugin> --dependency_out=${dependency_file} --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            MAIN_DEPENDENCY "${absolute_input}"
            DEPENDS protobuf.protoc_echo_plugin
            DEPFILE "${dependency_file}"
        )
    else()
        add_custom_command(
            OUTPUT ${generated_files}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:protobuf.protoc_echo_plugin> --dependency_out=${dependency_file} --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
            COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
            MAIN_DEPENDENCY "${absolute_input}"
            DEPENDS protobuf.protoc_echo_plugin
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

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-csharp-echo=$<TARGET_FILE:protobuf.protoc_echo_plugin_csharp> --csharp-echo_out="${generated_dir_echo}" ${absolute_input}
        BYPRODUCTS "${generated_dir_echo}/${source_base}.pb.cs"
    )
endfunction()

function(protocol_buffer_echo_java target input)
    protocol_buffer_echo_generator(${target} ${input})

    cmake_path(SET java_dir "generated/com/philips/emil/protobufEcho")
    cmake_path(SET generated_dir_echo "${java_dir}")

    cmake_path(ABSOLUTE_PATH java_dir BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE java_dir)
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    add_custom_command(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${EMIL_PROTOC_COMPILER_BINARY} ${protopath} --error_format=${error_format} --plugin=protoc-gen-java-echo=$<TARGET_FILE:protobuf.protoc_echo_plugin_java> --java-echo_out="${java_dir}" ${absolute_input}
        BYPRODUCTS "${generated_dir_echo}/${source_base}Services.java"
    )
endfunction()

function(protocol_buffer_echo_all target input)
    protocol_buffer_echo_cpp(${target} ${input})
    protocol_buffer_echo_csharp(${target} ${input})
    protocol_buffer_echo_java(${target} ${input})
endfunction()
