FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/philips-forks/mbedtls
    GIT_TAG 40b2dcdec50da1342425e5dfd215a12252a9b653 # v3.1.0 + 2869c67d6351c9bd62a8fc36f2654b5477cbdccf cherry-picked on top
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

set(DISABLE_PACKAGE_CONFIG_AND_INSTALL On CACHE INTERNAL "")
set(ENABLE_TESTING Off CACHE INTERNAL "")
set(ENABLE_PROGRAMS Off CACHE INTERNAL "")
set(GEN_FILES Off CACHE INTERNAL "")

FetchContent_MakeAvailable(mbedtls)

function(add_mbedtls_target_properties)
    foreach(target ${ARGN})
        target_compile_options(${target} PUBLIC
            -DMBEDTLS_CONFIG_FILE="mbedtls/mbedtls_emil_config.h"
        )

        target_include_directories(${target} PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/..>"
        )

        set_target_properties(${target} PROPERTIES FOLDER External/MbedTLS)
        install(TARGETS ${target} EXPORT emilCryptoTargets)
    endforeach()
endfunction()

add_mbedtls_target_properties(mbedcrypto mbedx509 mbedtls)
set_target_properties(apidoc lib PROPERTIES FOLDER External/MbedTLS)
