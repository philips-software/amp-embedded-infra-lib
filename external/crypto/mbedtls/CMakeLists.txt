FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/ARMmbed/mbedtls
    GIT_TAG v3.1.0
)

option(ENABLE_TESTING Off)
option(ENABLE_PROGRAMS Off)

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
    endforeach()
endfunction()

add_mbedtls_target_properties(mbedcrypto mbedx509 mbedtls)
set_target_properties(apidoc lib PROPERTIES FOLDER External/MbedTLS)
