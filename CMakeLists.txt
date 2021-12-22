cmake_minimum_required (VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(emil LANGUAGES C CXX VERSION 1.3.0)

set_directory_properties(PROPERTY USE_FOLDERS ON)

option(ENABLE_COVERAGE "Enable code coverage measurements" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(TARGET_BUILD_WIN ON)
    set(TARGET_ARCH_X86 ON)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(TARGET_BUILD_UNIX ON)
    set(TARGET_ARCH_X86 ON)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(TARGET_BUILD_DARWIN ON)
    set(TARGET_ARCH_X86 ON)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Generic")
    enable_language(ASM)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR
   "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" OR
   "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_compile_definitions(CCOLA_HOST_BUILD)

    include(CTest)
    include(GoogleTest)
    
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER External/GoogleTest)
    mark_as_advanced(BUILD_GMOCK BUILD_GTEST BUILD_SHARED_LIBS gmock_build_tests gtest_build_samples test_build_tests gtest_disable_pthreads gtest_force_shared_crt gtest_hide_internal_symbols)

    if (ENABLE_COVERAGE)
        add_compile_options(
            -g -O0 --coverage -fprofile-arcs -ftest-coverage -fno-inline
            $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-elide-constructors>
        )

        if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
            link_libraries(gcov)
        else()
            add_link_options(--coverage)
        endif()
    endif()
endif()

set(CCOLA_DIR "${CMAKE_SOURCE_DIR}/ccola")
include("${CCOLA_DIR}/ccola.inc")
include(version)

add_version_header_target(generated.version_header)

add_subdirectory(external/args)
add_subdirectory(external/crypto)
add_subdirectory(infra)
add_subdirectory(hal)
add_subdirectory(services)
add_subdirectory(external/protobuf-3.3.0)
add_subdirectory(external/protoc-3.3.0)
add_subdirectory(protobuf)
add_subdirectory(upgrade)
# add_subdirectory(lwip)
# add_subdirectory(examples)



# ccola_project()

#     if(CCOLA_ENABLE_COVERAGE)
#         ccola_enable_coverage_flags()
#     endif()

#     ccola_add_version_header()

#     ccola_subdirectories(
#         external/args
#         external/googletest
#         external/crypto
#         infra
#         hal
#         services
#         external/protobuf-3.3.0
#         external/protoc-3.3.0
#         protobuf
#         upgrade
#         lwip
#         examples
#     )

# ccola_end_project()