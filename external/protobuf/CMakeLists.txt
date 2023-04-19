if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG f0dc78d7e6e331b8c6bb2d5283e06aa26883ca7c # v21.12
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    add_compile_options(
        # Disable warning on invalid use of [[noreturn]] and on deprecation of sprintf
        # -Wno-invalid-noreturn should be removed when https://github.com/protocolbuffers/protobuf/issues/9817 is fixed
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations$<SEMICOLON>-Wno-invalid-noreturn>
        # Disable string overflow warning
        $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>
    )

    set(protobuf_BUILD_TESTS Off CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOBUF_BINARIES Off CACHE INTERNAL "")
    set(protobuf_BUILD_LIBPROTOC On CACHE INTERNAL "")
    set(protobuf_MSVC_STATIC_RUNTIME Off CACHE INTERNAL "")
    set(protobuf_WITH_ZLIB Off CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)

    function(add_protobuf_target_properties)
        foreach(target ${ARGN})
            set_target_properties(${target} PROPERTIES FOLDER External/Protobuf)
            if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
                # The "invalid noreturn" warning can't be ignored on GCC
                # Disable warning-as-error for these targets completely
                # This should be re-evaluated when https://github.com/protocolbuffers/protobuf/issues/9817 is fixed
                set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR Off)
            endif()
        endforeach()
    endfunction()

    add_protobuf_target_properties(libprotobuf libprotoc)
endif()
