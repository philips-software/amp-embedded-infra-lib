if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG 3d4adad5c4c4e6a6f9f038769b8c90716065b0e4 # v31.0
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
        # Increase template recursion depth and memory limit for MSVC to avoid ICE
        $<$<CXX_COMPILER_ID:MSVC>:/constexpr:depth2048>
        $<$<CXX_COMPILER_ID:MSVC>:/constexpr:backtrace10>
        # Additional MSVC flags to work around internal compiler errors
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        $<$<CXX_COMPILER_ID:MSVC>:/Zm1000>
        # Disable aggressive optimizations that can trigger ICE
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od>
        # Reduce template instantiation recursion to avoid ICE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
        # Use parallel compilation but limit processes to avoid memory issues
        $<$<CXX_COMPILER_ID:MSVC>:/MP4>
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
