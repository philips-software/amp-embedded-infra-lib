if (EMIL_HOST_BUILD)
    FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
        GIT_TAG b407e8416e3893036aee5af9a12bd9b6a0e2b2e6 # v29.3
    )

    set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

    add_compile_options(
        # Disable warning on invalid use of [[noreturn]] and on deprecation of sprintf
        # -Wno-invalid-noreturn should be removed when https://github.com/protocolbuffers/protobuf/issues/9817 is fixed
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wno-deprecated-declarations$<SEMICOLON>-Wno-invalid-noreturn>
        # Disable string overflow warning
        $<$<CXX_COMPILER_ID:GNU>:-Wno-stringop-overflow>
    )

    set(protobuf_VERBOSE On CACHE INTERNAL "")
    set(protobuf_BUILD_LIBPROTOC On CACHE INTERNAL "")
    set(protobuf_BUILD_PROTOC_BINARIES On CACHE INTERNAL "")
    set(protobuf_MSVC_STATIC_RUNTIME Off CACHE INTERNAL "")
    set(protobuf_WITH_ZLIB Off CACHE INTERNAL "")
    set(protobuf_BUILD_TESTS Off CACHE INTERNAL "")
    set(protobuf_INSTALL Off CACHE INTERNAL "")

    set(ABSL_PROPAGATE_CXX_STD On CACHE INTERNAL "")    
    set(ABSL_FIND_GOOGLETEST On CACHE INTERNAL "")
    set(ABSL_USE_EXTERNAL_GOOGLETEST Off CACHE INTERNAL "")
    set(ABSL_BUILD_TESTING Off CACHE INTERNAL "")
    set(ABSL_BUILD_TEST_HELPERS Off CACHE INTERNAL "")
    set(ABSL_ENABLE_INSTALL Off CACHE INTERNAL "")

    FetchContent_MakeAvailable(protobuf)

    
    function(apply_workaround_to_build_protobuf)        
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(absl_strings PRIVATE -Wno-array-bounds)
        endif()

        if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            # Workaround for warning C4141: 'inline': used more than once
            set_target_properties(utf8_validity PROPERTIES COMPILE_WARNING_AS_ERROR Off)
            set_target_properties(utf8_range PROPERTIES COMPILE_WARNING_AS_ERROR Off)
        endif()
    endfunction()

    function(add_protobuf_target_properties)
        foreach(target ${ARGN})
            set_target_properties(${target} PROPERTIES FOLDER External/Protobuf)
            if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                # The "invalid noreturn" warning can't be ignored on GCC
                # Disable warning-as-error for these targets completely
                # This should be re-evaluated when https://github.com/protocolbuffers/protobuf/issues/9817 is fixed
                set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR Off)
            endif()
        endforeach()
    endfunction()

    apply_workaround_to_build_protobuf()
    add_protobuf_target_properties(libprotobuf libprotoc)
endif()
