cmake_minimum_required(VERSION 3.24)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(emil LANGUAGES C CXX VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED On)

set_directory_properties(PROPERTY USE_FOLDERS On)

include(CMakePackageConfigHelpers)
include(emil_clang_tools)
include(emil_docker_tools)
include(emil_test_helpers)
include(emil_version_generator)
include(emil_xsltproc)
include(FetchContent)
include(GNUInstallDirs)  # included to supply proper defaults to install(...) commands

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(EMIL_STANDALONE On)
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

option(CMAKE_COMPILE_WARNING_AS_ERROR "Enable warnings-as-error" ON)
option(EMIL_BUILD_TESTS "Enable building the tests" ${EMIL_DEFAULTOPT})
option(EMIL_BUILD_EXAMPLES "Enable building the examples" ${EMIL_DEFAULTOPT})
option(EMIL_ENABLE_FUZZING "Enable building the fuzzing targets" Off)
option(EMIL_ENABLE_DOCKER_TOOLS "Enable shift-left tools (e.g. linters, formatters) that are run using Docker" On)

# The next few options control the install and export behaviour.
# When package config generation is enabled the prefix can be set
# with the EMIL_PACKAGE_CONFIG_EXPORT_NAMESPACE (default emil::).
# The EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE controls the namespace
# used for all targets that are used during the build (mostly for
# code generation). In the default scenario this is empty, using
# targets of the current build. When doing a 'cross-build' this
# should be set to the export name of the 'host' build.
option(EMIL_GENERATE_PACKAGE_CONFIG "Enable generation of package configuration and install files" On)
set(EMIL_PACKAGE_CONFIG_EXPORT_NAMESPACE "${PROJECT_NAME}::" CACHE STRING "Namespace used for exported targets")
if (CMAKE_CROSSCOMPILING)
    set(EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE "${PROJECT_NAME}::" CACHE STRING "Namespace used to reference imported targets")
else()
    set(EMIL_PACKAGE_CONFIG_IMPORT_NAMESPACE "" CACHE STRING "Namespace used to reference imported targets")
endif()

option(EMIL_INCLUDE_MBEDTLS "Include MbedTLS as part of EmIL" On)

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

add_version_header_target(generated.version_header)

add_subdirectory(external)
add_subdirectory(infra)
add_subdirectory(hal)
add_subdirectory(services)
add_subdirectory(protobuf)
add_subdirectory(upgrade)
add_subdirectory(lwip)
if (EMIL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (EMIL_GENERATE_PACKAGE_CONFIG)
    function(generate_export_targets name)
        # Generate and install export file
        install(EXPORT emil${name}Targets
            FILE emil${name}Targets.cmake
            NAMESPACE ${EMIL_PACKAGE_CONFIG_EXPORT_NAMESPACE}
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
        )

        # Generate the export targets for the build tree
        export(EXPORT emil${name}Targets
            FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/emil${name}Targets.cmake"
            NAMESPACE ${EMIL_PACKAGE_CONFIG_EXPORT_NAMESPACE}
        )
    endfunction()

    foreach(target IN ITEMS Crypto Infra Hal Lwip Protobuf Services Upgrade)
        generate_export_targets(${target})
    endforeach()

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY AnyNewerVersion
        # When cross-compiling for a 32-bit architecture and re-using host tooling from a 64-bit architecture
        # ARCH_INDEPENDENT is necessary here. See: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html.
        ARCH_INDEPENDENT
    )

    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    set(CPACK_SOURCE_IGNORE_FILES ".vs/;.git/;Build/")
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR "Koninklijke Philips N.V")
    set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
    set(CPACK_DEBIAN_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_VENDOR}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    include(CPack)
endif()
