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

        target_compile_options(${target} PUBLIC
            # see https://github.com/Mbed-TLS/mbedtls/pull/6966
            # mbedtls sets the -Wdocumentation flag, which is throwing warnings
            # since clang-15
            $<$<CXX_COMPILER_ID:Clang>:-Wno-documentation -Wno-conversion>
            $<$<CXX_COMPILER_ID:AppleClang>:-Wno-documentation -Wno-conversion>
            # Warning treated as error when implicit conversion within mbedtls
            $<$<CXX_COMPILER_ID:GNU>:-Wno-conversion>
            # Warnings treated as error since clang-22
            $<$<CXX_COMPILER_ID:Clang>:$<$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,22.0.0>:-Wno-uninitialized-const-pointer>>
            $<$<CXX_COMPILER_ID:Clang>:$<$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,22.0.0>:-Wno-unterminated-string-initialization>>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4244 /wd4267>
        )

        target_include_directories(${target} PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
        )

        set_target_properties(${target} PROPERTIES FOLDER External/MbedTLS)
    endforeach()
endfunction()

add_mbedtls_target_properties(mbedcrypto mbedx509 mbedtls)
set_target_properties(apidoc lib PROPERTIES FOLDER External/MbedTLS)

# Without optimization bignum_core fails to compile due to lack of assembly registers
target_compile_options(mbedcrypto PRIVATE $<$<CXX_COMPILER_ID:GNU>:$<$<CONFIG:Debug>:-O2>>)
