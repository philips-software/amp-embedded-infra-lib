add_library(upgrade.boot_loader ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(upgrade.boot_loader PUBLIC
    crypto.micro_ecc
    crypto.tiny_aes128
    hal.synchronous_interfaces
    infra.syntax
    services.synchronous_util
    upgrade.pack
)

target_sources(upgrade.boot_loader PRIVATE
    Decryptor.hpp
    DecryptorAesTiny.cpp
    DecryptorAesTiny.hpp
    DecryptorNone.cpp
    DecryptorNone.hpp
    ImageUpgrader.cpp
    ImageUpgrader.hpp
    ImageUpgraderEraseSectors.cpp
    ImageUpgraderEraseSectors.hpp
    ImageUpgraderFlash.cpp
    ImageUpgraderFlash.hpp
    ImageUpgraderSkip.cpp
    ImageUpgraderSkip.hpp
    PackUpgrader.cpp
    PackUpgrader.hpp
    SecondStageToRamLoader.cpp
    SecondStageToRamLoader.hpp
    UpgradePackLoader.cpp
    UpgradePackLoader.hpp
    Verifier.hpp
)

if (EMIL_INCLUDE_MBEDTLS OR NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
    if (NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
        target_link_libraries(upgrade.boot_loader PUBLIC
            ${EMIL_EXTERNAL_MBEDTLS_TARGET}
        )
    else()
        target_link_libraries(upgrade.boot_loader PUBLIC
            mbedcrypto
        )
    endif()

    target_sources(upgrade.boot_loader PRIVATE
        DecryptorAesMbedTls.cpp
        DecryptorAesMbedTls.hpp
        VerifierEcDsa.cpp
        VerifierEcDsa.hpp
        VerifierHashOnly.cpp
        VerifierHashOnly.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
