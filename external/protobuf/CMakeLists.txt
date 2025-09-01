if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG 674a00f47745c24d854e1d4d75786cf847d2bcb0 # some main after v32.0-rc2
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    add_compile_options(
        # Disable warning on invalid use of [[noreturn]] and on deprecation of sprintf
        # -Wno-invalid-noreturn should be removed when https://github.com/protocolbuffers/protobuf/issues/9817 is fixed
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations$<SEMICOLON>-Wno-invalid-noreturn$<SEMICOLON>-Winvalid-offsetof>
        # Disable string overflow warning
        $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>
        # Disable warning not all control paths return a value
        $<$<CXX_COMPILER_ID:MSVC>:/wd4715>
        # Disable warning differs in levels of indirection
        $<$<CXX_COMPILER_ID:MSVC>:/wd4047>
        # Disable warning unnamed type definition in parentheses
        $<$<CXX_COMPILER_ID:MSVC>:/wd4116>
        # Disable warning inline used more than once
        $<$<CXX_COMPILER_ID:MSVC>:/wd4141>
    )

    set(protobuf_BUILD_TESTS Off CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOBUF_BINARIES On CACHE INTERNAL "")
    set(protobuf_BUILD_LIBPROTOC On CACHE INTERNAL "")
    set(protobuf_MSVC_STATIC_RUNTIME Off CACHE INTERNAL "")
    set(protobuf_WITH_ZLIB Off CACHE INTERNAL "")

    set(old_build_testing ${BUILD_TESTING})

    FetchContent_MakeAvailable(protobuf)

    # Absl unconditionally includes CTest, so restore it to its previous value
    set(BUILD_TESTING "${old_build_testing}" CACHE INTERNAL "")

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
