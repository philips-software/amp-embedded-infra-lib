if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG f0dc78d7e6e331b8c6bb2d5283e06aa26883ca7c # v21.12
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    set(protobuf_BUILD_TESTS Off CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOBUF_BINARIES Off CACHE INTERNAL "")
    set(protobuf_BUILD_LIBPROTOC On CACHE INTERNAL "")
    set(protobuf_MSVC_STATIC_RUNTIME Off CACHE INTERNAL "")
    set(protobuf_WITH_ZLIB Off CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)

    if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|AppleClang")
        # Disable warning on invalid use of [[noreturn]]
        add_compile_options(-Wno-invalid-noreturn)
        set_source_files_properties(${protobuf_SOURCE_DIR}/src/google/protobuf/stubs/strutil.cc PROPERTIES COMPILE_OPTIONS -Wno-deprecated-declarations)
    endif()

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
