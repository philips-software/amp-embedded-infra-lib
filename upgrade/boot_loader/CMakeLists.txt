add_library(upgrade.boot_loader ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(upgrade.boot_loader PUBLIC
    mbedcrypto
    crypto.micro_ecc
    crypto.tiny_aes128
    hal.synchronous_interfaces
    infra.syntax
    services.synchronous_util
    upgrade.pack
)

target_sources(upgrade.boot_loader PRIVATE
    Decryptor.hpp
    DecryptorAesMbedTls.cpp
    DecryptorAesMbedTls.hpp
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
    VerifierEcDsa.cpp
    VerifierEcDsa.hpp
    VerifierHashOnly.cpp
    VerifierHashOnly.hpp
)

add_subdirectory(test)
add_subdirectory(test_doubles)
