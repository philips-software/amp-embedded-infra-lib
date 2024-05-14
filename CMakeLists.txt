cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}")

if (APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "OS X target architectures")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
endif()

project(emil LANGUAGES C CXX VERSION 6.1.0) # x-release-please-version

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)

set_directory_properties(PROPERTY USE_FOLDERS On)

include(CMakePackageConfigHelpers)
include(emil_antora)
include(emil_build_for)
include(emil_folderize)
include(emil_clang_tools)
include(emil_docker_tools)
include(emil_generate_artifacts)
include(emil_test_helpers)
include(emil_version_generator)
include(emil_xsltproc)
include(FetchContent)
include(GNUInstallDirs) # included to supply proper defaults to install(...) commands

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(EMIL_STANDALONE On)
endif()

if (MINGW)
    # When compiling with MINGW, dynamically linking to the standard C++ library results in linking to libstdc++-6.dll.
    # The libstdc++-6.dll found during runtime is often different than the one with which the application is compiled with,
    # so instead of linking dynamically, we statically link to the standard C++ library.
    add_link_options("-static-libstdc++")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EMIL_BUILD_WIN On)
    set(EMIL_HOST_BUILD On)

    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/EHsc>)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(EMIL_BUILD_UNIX On)
    set(EMIL_HOST_BUILD On)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(EMIL_BUILD_DARWIN On)
    set(EMIL_HOST_BUILD On)

    # Suppress warnings about *.a files having no symbols on Darwin.
    # The warning is emitted when *.cpp.o contains no operational code.
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Generic")
    enable_language(ASM)
endif()

# When building EmIL by itself the examples and tests will be built
# by-default. When consuming EmIL as a dependency the tests and
# examples are disabled. This behavior can be overridden by below
# options.
if (EMIL_STANDALONE AND EMIL_HOST_BUILD)
    set(EMIL_DEFAULTOPT On)
else()
    set(EMIL_DEFAULTOPT Off)
endif()

# When building EmIL by itself do not exclude any targets from all
# Libraries will only be excluded from all when EmIL is a consumed as a dependency.
if (EMIL_STANDALONE)
    set(EMIL_EXCLUDE_FROM_ALL "")
else()
    set(EMIL_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
endif()

option(EMIL_BUILD_TESTS "Enable building the tests" ${EMIL_DEFAULTOPT})
option(EMIL_BUILD_EXAMPLES "Enable building the examples" ${EMIL_DEFAULTOPT})
option(EMIL_ENABLE_FUZZING "Enable building the fuzzing targets" Off)
option(EMIL_ENABLE_DOCKER_TOOLS "Enable shift-left tools (e.g. linters, formatters) that are run using Docker" On)
option(EMIL_GENERATE_PACKAGE_CONFIG "Enable generation of package configuration and install files" ${EMIL_HOST_BUILD})
option(EMIL_ENABLE_TRACING "Enable Tracing" On)
option(EMIL_BUILD_ECHO_COMPILERS "Build and install the Protobuf ECHO compilers" ${EMIL_DEFAULTOPT})

# Enable or disable optional features.
option(EMIL_INCLUDE_MBEDTLS "Include MbedTLS as part of EmIL" On)
option(EMIL_INCLUDE_FREERTOS "Include FreeRTOS as part of EmIL" Off)
option(EMIL_INCLUDE_THREADX "Include ThreadX as part of EmIL (Incomplete, experimental)" Off)
option(EMIL_INCLUDE_SEGGER_RTT "Include support for Segger RTT" Off)
set(EMIL_EXTERNAL_LWIP_TARGET "" CACHE STRING "Specify an external LWIP target")

if (EMIL_ENABLE_DOCKER_TOOLS)
    emil_enable_docker_tools()
endif()

if (EMIL_HOST_BUILD)
    add_compile_definitions(EMIL_HOST_BUILD)

    if (EMIL_BUILD_TESTS)
        include(CTest)
        emil_enable_testing()
    endif()

    if (EMIL_ENABLE_FUZZING)
        set(EMIL_EXCLUDE_FROM_ALL "EXCLUDE_FROM_ALL")
        emil_enable_fuzzing()
    endif()
endif()

if (NOT EMIL_STANDALONE)
    set(EMIL_HOST_BUILD "${EMIL_HOST_BUILD}" PARENT_SCOPE)
    set(EMIL_BUILD_WIN "${EMIL_BUILD_WIN}" PARENT_SCOPE)
    set(EMIL_BUILD_UNIX "${EMIL_BUILD_UNIX}" PARENT_SCOPE)
    set(EMIL_BUILD_DARWIN "${EMIL_BUILD_DARWIN}" PARENT_SCOPE)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Suppress warnings about parameter passing for argument of type '...' changed in GCC 7.1
    add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-psabi>)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-psabi>)
endif()

add_version_header_target(generated.version_header)

add_subdirectory(external)
add_subdirectory(infra)
add_subdirectory(hal)
add_subdirectory(protobuf)
add_subdirectory(services)
add_subdirectory(upgrade)
add_subdirectory(lwip)
add_subdirectory(osal)
if (EMIL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

emil_clangformat_directories(emil DIRECTORIES .)

if (EMIL_STANDALONE)
    emil_generate_antora_docs_target(antora-playbook-branch.yml)
    emil_folderize_all_targets()
endif()

if (EMIL_BUILD_ECHO_COMPILERS)
    function(generate_export_targets name)
        install(EXPORT emil${name}Targets
            FILE emil${name}Targets.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
        )
    endfunction()

    foreach(target IN ITEMS Protobuf)
        generate_export_targets(${target})
    endforeach()

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
        # When cross-compiling for a 32-bit architecture and re-using host tooling from a 64-bit architecture
        # ARCH_INDEPENDENT is necessary here. See: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html.
        ARCH_INDEPENDENT
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

endif()

set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_SOURCE_IGNORE_FILES ".vs/;.git/;build/")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Koninklijke Philips N.V")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)
