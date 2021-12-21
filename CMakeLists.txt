cmake_minimum_required (VERSION 3.20)

project(emil VERSION 1.3.0)

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

    # Now simply link against gtest or gtest_main as needed. Eg
    # add_executable(example example.cpp)
    # target_link_libraries(example gtest_main)
    # add_test(NAME example_test COMMAND example)
endif()

set(CCOLA_DIR "${CMAKE_SOURCE_DIR}/ccola")
set(CCOLA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
include("${CCOLA_DIR}/ccola.inc")

add_subdirectory(external/args)
add_subdirectory(external/crypto)
add_subdirectory(infra)
add_subdirectory(hal)
add_subdirectory(services)
add_subdirectory(external/protobuf-3.3.0)
add_subdirectory(external/protoc-3.3.0)
add_subdirectory(protobuf)
# add_subdirectory(upgrade)
# add_subdirectory(lwip)
# add_subdirectory(examples)


# set(CMAKE_CXX_STANDARD_REQUIRED true)

# if(NOT CCOLA_DIR)
#     set(CCOLA_DIR "${CMAKE_SOURCE_DIR}/ccola")
# endif()

# if(NOT CCOLA_INSTALL_DIR)
# 	set(CCOLA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
# endif()

# include("${CCOLA_DIR}/ccola.inc")

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