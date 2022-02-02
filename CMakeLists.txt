cmake_minimum_required (VERSION 3.21)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

project(emil LANGUAGES C CXX VERSION 2.0.0)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(EMIL_STANDALONE On)
endif()

include(FetchContent)
include(CMakeDependentOption)

# When building EmIL by itself the examples and tests will be built
# by-default. When consuming EmIL as a dependency the tests and
# examples are disabled. This behavior can be overridden by below
# options.
if (EMIL_STANDALONE)
    set(EMIL_DEFAULTOPT On)
else()
    set(EMIL_DEFAULTOPT Off)
endif()

option(EMIL_BUILD_TESTS "Enable build of the tests" ${EMIL_DEFAULTOPT})
option(EMIL_BUILD_EXAMPLES "Enable build of the examples" ${EMIL_DEFAULTOPT})
cmake_dependent_option(EMIL_ENABLE_COVERAGE "Enable compiler flags for code coverage measurements" Off "EMIL_BUILD_TESTS" Off)

option(EMIL_BUILD_PROTOBUF "Enable build of protobuf and echo-rpc library." On)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set_directory_properties(PROPERTY USE_FOLDERS ON)

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

    # Suppress warnings about *.a files having no symbols on Darwin.
    # The warning is emitted when *.cpp.o contains no operational code.
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Generic")
    enable_language(ASM)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows" OR
   "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin" OR
   "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_compile_definitions(EMIL_HOST_BUILD)

    if (EMIL_BUILD_TESTS)
        include(CTest)
        include(GoogleTest)
    
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
    endif()
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)  # included to supply proper defaults to install(...) commands
include(version)

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

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/emilConfigVersion.cmake"
    COMPATIBILITY AnyNewerVersion
    # When cross-compiling for a 32-bit architecture and re-using host tooling from a 64-bit architecture
    # ARCH_INDEPENDENT is necessary here. See: https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html.
    ARCH_INDEPENDENT
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_BINARY_DIR}/emilConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_BINARY_DIR}/emilConfig.cmake"
    "${CMAKE_BINARY_DIR}/emilConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/emil
)
