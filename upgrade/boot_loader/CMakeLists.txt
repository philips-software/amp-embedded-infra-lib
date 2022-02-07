add_library(upgrade.boot_loader EXCLUDE_FROM_ALL STATIC)
add_library(emil::upgrade.boot_loader ALIAS upgrade.boot_loader)
install(TARGETS upgrade.boot_loader EXPORT emilUpgradeTargets)

target_link_libraries(upgrade.boot_loader PUBLIC
    mbedcrypto
    crypto.micro_ecc
    crypto.tiny_aes128
    hal.synchronous_interfaces
    infra.stream
    infra.syntax
    infra.util
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
    Verifier.hpp
    VerifierEcDsa.cpp
    VerifierEcDsa.hpp
    VerifierHashOnly.cpp
    VerifierHashOnly.hpp
    VerifierNone.cpp
    VerifierNone.hpp
)

if (BUILD_TESTING)
    add_subdirectory(test)
    add_subdirectory(test_doubles)
endif()
