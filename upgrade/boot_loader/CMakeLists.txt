add_library(upgrade.boot_loader EXCLUDE_FROM_ALL STATIC)

target_link_libraries(upgrade.boot_loader
    PUBLIC
    crypto.mbed_tls_minimal
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
    DecryptorXtea.cpp
    DecryptorXtea.hpp
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
    VerifierNone.cpp
    VerifierNone.hpp
    VerifierRsa.cpp
    VerifierRsa.hpp
)

add_subdirectory(test)
add_subdirectory(test_doubles)
