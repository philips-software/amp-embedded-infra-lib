FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/ARMmbed/mbedtls
    GIT_TAG bff88ab0862314bca40fbb5976427abe093c9b7b
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
