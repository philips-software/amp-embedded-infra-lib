if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG edaa823d8b36a8656d7b2b9241b7d0bfe50af878 # v33.4
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    add_compile_options(
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
        # Disable warning Operands are different enum types
        $<$<CXX_COMPILER_ID:MSVC>:/wd5287>
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
            if (${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "AppleClang")
                # Disable warnings about weak() being deprecated
                target_compile_options(${target} PUBLIC -Wno-deprecated-declarations)
            endif()
            if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" AND "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" MATCHES "MSVC")
                # unused-command-line-argument: /MP option is set by the library when building with MSVC, but the flag has no effect on clang-cl
                target_compile_options(${target} PUBLIC -Wno-unused-command-line-argument -Wno-invalid-offsetof)
            endif()
        endforeach()
    endfunction()

    add_protobuf_target_properties(libprotobuf libprotoc libupb)
endif()
