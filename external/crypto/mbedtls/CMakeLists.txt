FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls
    GIT_TAG 869298bffeea13b205343361b7a7daf2b210e33d # v3.2.1
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
        target_compile_options(${target} PUBLIC
            -DMBEDTLS_CONFIG_FILE="mbedtls/mbedtls_emil_config.h"
            # see https://github.com/Mbed-TLS/mbedtls/pull/6966
            # mbedtls sets the -Wdocumentation flag, which is throwing warnings
            # since clang-15
            $<$<CXX_COMPILER_ID:Clang>:-Wno-documentation>
        )

        target_include_directories(${target} PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
        )

        set_target_properties(${target} PROPERTIES FOLDER External/MbedTLS)
    endforeach()
endfunction()

add_mbedtls_target_properties(mbedcrypto mbedx509 mbedtls)
set_target_properties(apidoc lib PROPERTIES FOLDER External/MbedTLS)
