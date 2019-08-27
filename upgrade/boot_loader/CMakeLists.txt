ccola_component(boot_loader static_library)

    ccola_sources(
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
        ImageUpgraderInternalFlash.cpp
        ImageUpgraderInternalFlash.hpp
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

    ccola_dependencies(
        pack
        infra.util
        infra.stream
        infra.syntax
        crypto.mbed_tls_minimal
        crypto.micro_ecc
        crypto.tiny_aes128
        hal.synchronous_interfaces
        services.synchronous_util
    )

ccola_end_component()

ccola_subdirectories(
    test_doubles
    test
)
