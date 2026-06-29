FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls
    GIT_TAG 107ea89daaefb9867ea9121002fbbdf926780e98 # v3.6.2
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

set(DISABLE_PACKAGE_CONFIG_AND_INSTALL On CACHE INTERNAL "")
set(ENABLE_TESTING Off CACHE INTERNAL "")
set(ENABLE_PROGRAMS Off CACHE INTERNAL "")
set(GEN_FILES Off CACHE INTERNAL "")

# mbedtls uses CMAKE_C_COMPILER_ID to determine the type of compiler used.
# When compiling with clang-cl, mbedtls thinks the clang compiler
# is used, and sets flags according to what clang expects as arguments.
# However, clang-cl expects MSVC like arguments. Therefore we have to trick mbedtls
# in to thinking MSVC is being used as a compiler.
# After making mbedtls available the original COMPILER_ID will be restored
set(ORIGINAL_C_COMPILER_ID ${CMAKE_C_COMPILER_ID})
set(ORIGINAL_CXX_COMPILER_ID ${CMAKE_CXX_COMPILER_ID})

if (CMAKE_C_SIMULATE_ID)
    set(CMAKE_C_COMPILER_ID ${CMAKE_C_SIMULATE_ID})
endif()

if (CMAKE_CXX_SIMULATE_ID)
    set(CMAKE_CXX_COMPILER_ID ${CMAKE_CXX_SIMULATE_ID})
endif()

FetchContent_MakeAvailable(mbedtls)

set(CMAKE_C_COMPILER_ID ${ORIGINAL_C_COMPILER_ID})
set(CMAKE_CXX_COMPILER_ID ${ORIGINAL_CXX_COMPILER_ID})

function(add_mbedtls_target_properties)
    foreach(target ${ARGN})
        target_compile_definitions(${target} PUBLIC
            MBEDTLS_CONFIG_FILE="mbedtls/mbedtls_emil_config.h"
        )

        # mbedtls is a C library whose targets are also consumed by C++ code.
        # The $<C_COMPILER_ID> / $<CXX_COMPILER_ID> generator expressions are
        # unreliable in this scope: just above, the compiler id is temporarily
        # spoofed to MSVC (CMAKE_C_SIMULATE_ID) so that mbedtls configures itself
        # for a clang-cl / MSVC-style command line. As a result the genex-based
        # branches never reach mbedtls' own C sources (e.g. x509_crt.c). To make
        # the warning suppressions actually land on the compile lines we select
        # the flags at configure time based on the (restored) detected compiler.
        set(_mbedtls_warning_suppressions)

        if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
            # see https://github.com/Mbed-TLS/mbedtls/pull/6966
            # mbedtls sets the -Wdocumentation flag, which throws warnings since clang-15
            list(APPEND _mbedtls_warning_suppressions -Wno-documentation -Wno-conversion -Wno-uninitialized-const-pointer -Wno-unterminated-string-initialization)
        endif()

        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            # -Wuninitialized-const-pointer / -Wunterminated-string-initialization
            # are treated as errors since clang-22
            if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 22.0.0)
                list(APPEND _mbedtls_warning_suppressions -Wno-uninitialized-const-pointer -Wno-unterminated-string-initialization)
            endif()
        elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            # Warning treated as error when implicit conversion within mbedtls
            list(APPEND _mbedtls_warning_suppressions -Wno-conversion)
            # Warning treated as error since gcc-14
            if(CMAKE_C_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0.0)
                list(APPEND _mbedtls_warning_suppressions -Wno-unterminated-string-initialization)
            endif()
        elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
            list(APPEND _mbedtls_warning_suppressions /wd4244 /wd4267)
        endif()

        if(_mbedtls_warning_suppressions)
            target_compile_options(${target} PUBLIC ${_mbedtls_warning_suppressions})
        endif()

        target_include_directories(${target} PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
        )

        set_target_properties(${target} PROPERTIES FOLDER External/MbedTLS)
    endforeach()
endfunction()

add_mbedtls_target_properties(mbedcrypto mbedx509 mbedtls)
set_target_properties(apidoc lib PROPERTIES FOLDER External/MbedTLS)

# Without optimization bignum_core fails to compile due to lack of assembly registers
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(mbedcrypto PRIVATE $<$<CONFIG:Debug,DebugOptimized>:-O2>)
endif()
