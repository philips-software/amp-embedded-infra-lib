if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG 90b73ac3f0b10320315c2ca0d03a5a9b095d2f66 # v21.9
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|AppleClang")
        # Disable warning on invalid use of [[noreturn]]
        add_compile_options(-Wno-invalid-noreturn)
    endif()

    set(protobuf_BUILD_TESTS Off CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOBUF_BINARIES Off CACHE INTERNAL "")
    set(protobuf_BUILD_LIBPROTOC On CACHE INTERNAL "")
    set(protobuf_WITH_ZLIB Off CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)

    function(add_protobuf_target_properties)
        foreach(target ${ARGN})
            set_target_properties(${target} PROPERTIES FOLDER External/Protobuf)
            if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
                # There are some warnings that can't be ignored on GCC
                # Disable warning-as-error for these targets completely
                set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR Off)
            endif()

            install(TARGETS ${target} EXPORT emilProtobufTargets)
        endforeach()
    endfunction()

    add_protobuf_target_properties(libprotobuf libprotoc)
endif()
