set(PROTOC_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "protoc-3.3.0 directory")

function(protocol_buffer_cpp target input)

    cmake_path(ABSOLUTE_PATH input NORMALIZE OUTPUT_VARIABLE absolute_input)
    cmake_path(GET absolute_input STEM source_base)
    cmake_path(GET absolute_input PARENT_PATH absolute_directory_input)
    cmake_path(SET generated_dir_cpp "generated/proto_cpp")
    cmake_path(ABSOLUTE_PATH generated_dir_cpp BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE generated_dir_cpp)

    set(protopath)
    foreach(dependency ${PROTOBUF_DEPENDENCIES})
        emil_read_public_property("${dependency}" proto paths)
        foreach(path ${paths})
            list(APPEND protopath --proto_path ${path})
        endforeach()
    endforeach()

    get_target_property(PROTOBUF_DIR emil::protobuf.libprotobuf SOURCE_DIR)

    foreach(dependency_path "${absolute_directory_input}" "${PROTOBUF_DIR}/src/google/protobuf" "${PROTOC_DIR}/../../protobuf/echo_attributes")
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

    set(generated_files "${generated_dir_cpp}/${source_base}.pb.cc" "${generated_dir_cpp}/${source_base}.pb.h")

    add_custom_command(
        OUTPUT ${generated_files}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_dir_cpp}
        COMMAND ${protoc_compiler_binary} ${protopath} --error_format=${error_format} --cpp_out="${generated_dir_cpp}" ${absolute_input}
        DEPENDS "${absolute_input}"
    )

    target_sources(${target} PRIVATE
        ${generated_files}
    )

    target_include_directories(${target} PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${generated_dir_cpp}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    target_link_libraries(${target} PUBLIC
        protobuf.libprotobuf
    )

endfunction()
