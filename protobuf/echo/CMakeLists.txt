add_library(protobuf.echo EXCLUDE_FROM_ALL STATIC)
add_library(emil::protobuf.echo ALIAS protobuf.echo)
install(TARGETS protobuf.echo EXPORT emilProtobufTargets)

target_link_libraries(protobuf.echo PUBLIC
    infra.stream
    infra.syntax
    services.network
)

target_sources(protobuf.echo PRIVATE
    Echo.cpp
    Echo.hpp
    EchoInstantiation.hpp
)

if (BUILD_TESTING)
    add_subdirectory(test_helper)
endif()

function(protocol_buffer_echo_cpp target input)

    if (CMAKE_CROSSCOMPILING)
        find_package(emil COMPONENTS ProtobufHost)
    endif()

    cmake_path(ABSOLUTE_PATH input NORMALIZE OUTPUT_VARIABLE absolute_input)
    cmake_path(GET absolute_input STEM source_base)
    cmake_path(GET absolute_input PARENT_PATH absolute_directory_input)
    cmake_path(SET generated_dir_echo "generated/echo")
    cmake_path(ABSOLUTE_PATH generated_dir_echo BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_echo)

    set(protopath)
    get_target_property(protobuf_dependencies ${target} PROTOBUF_DEPENDENCIES)
    if (protobuf_dependencies)
        foreach(dependency ${protobuf_dependencies})
            get_target_property(path ${dependency} SOURCE_DIR)
            list(APPEND protopath --proto_path ${path})
        endforeach()
    endif()

    get_target_property(ECHO_ATTRIBUTES_DIR emil::protobuf.echo_attributes SOURCE_DIR)
    get_target_property(PROTOBUF_DIR emil::protobuf.libprotobuf SOURCE_DIR)

    foreach(dependency_path "${absolute_directory_input}" "${PROTOBUF_DIR}/src/google/protobuf" "${ECHO_ATTRIBUTES_DIR}")
        foreach(path ${dependency_path})
            list(APPEND protopath "--proto_path=${path}")
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

    set(generated_files "${generated_dir_echo}/${source_base}.pb.cpp" "${generated_dir_echo}/${source_base}.pb.hpp" "${generated_dir_echo}/${source_base}.pb")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_echo}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:emil::protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --plugin=protoc-gen-cpp-infra=$<TARGET_FILE:emil::protobuf.protoc_echo_plugin> --cpp-infra_out="${generated_dir_echo}" ${absolute_input}
        COMMAND ${CMAKE_COMMAND} -E env "\"PATH=$<TARGET_FILE_DIR:emil::protobuf.libprotobuf>;\"" ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --descriptor_set_out="${generated_dir_echo}/${source_base}.pb" --include_imports ${absolute_input}
        MAIN_DEPENDENCY "${absolute_input}"
        DEPENDS emil::protobuf.protoc_echo_plugin
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
