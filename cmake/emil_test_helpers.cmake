option(EMIL_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off)
option(EMIL_ENABLE_MUTATION_TESTING "Enable compiler flags for mutation testing" Off)

function(emil_enable_testing)
    include(GoogleTest)

    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt On CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER External/GoogleTest)
    mark_as_advanced(BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS gmock_build_tests gtest_build_samples test_build_tests gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols)

    if (EMIL_ENABLE_COVERAGE)
        add_compile_options(
            -g -O0 --coverage -fprofile-arcs -ftest-coverage -fno-inline
            $<$<COMPILE_LANGUAGE:CXX>:-fno-elide-constructors>
        )

        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
            link_libraries(gcov)
        else()
            add_link_options(--coverage)
        endif()
    endif()

    if (EMIL_ENABLE_MUTATION_TESTING)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_compile_options(
                -g -O0 -grecord-command-line -fprofile-instr-generate -fcoverage-mapping
                -fexperimental-new-pass-manager -fpass-plugin=/usr/lib/mull-ir-frontend-12
            )

            add_link_options(-fprofile-instr-generate)
        else()
            message(FATAL_ERROR "Mutation testing is currently only supported for Clang/LLVM; not for ${CMAKE_CXX_COMPILER_ID}")
        endif()
    endif()
endfunction()

function(emil_add_test target)
    if (EMIL_ENABLE_MUTATION_TESTING)
        add_test(NAME ${target} COMMAND mull-runner-12 $<TARGET_FILE:${target}>)
    else()
        add_test(NAME ${target} COMMAND ${target})
    endif()
endfunction()
