ccola_component(pack_builder x86: static_library other: noncompile)

    ccola_sources(
        BinaryObject.cpp
        BuildReferenceUpgradePack.cpp
        BuildUpgradePack.cpp
        ImageAuthenticatorHmac.cpp
        ImageEncryptorAes.cpp
        ImageEncryptorNone.cpp
        ImageEncryptorXtea.cpp
        ImageSecurityXteaHmac.cpp
        ImageSignerEcDsa.cpp
        ImageSignerRsa.cpp
        Input.cpp
        InputBinary.cpp
        InputHex.cpp
        RandomNumberGenerator.cpp
        UpgradePackBuilder.cpp
        UpgradePackInputFactory.cpp
        BinaryObject.hpp
        BuildReferenceUpgradePack.hpp
        BuildUpgradePack.hpp
        ImageAUthenticatorHmac.hpp
        ImageEncryptorAes.hpp
        ImageEncryptorNone.hpp
        ImageEncryptorXtea.hpp
        ImageSecurity.hpp
        ImageSecurityXteaHmac.hpp
        ImageSigner.hpp
        ImageSignerEcDsa.hpp
        ImageSignerRsa.hpp
        Input.hpp
        InputBinary.hpp
        InputFactory.hpp
        InputHex.hpp
        RandomNumberGenerator.hpp
        SparseVector.hpp
        UpgradePackBuilder.hpp
        UpgradePackInputFactory.hpp
        UpgradePackConfigParser.cpp
        UpgradePackConfigParser.hpp
    )

    ccola_dependencies(
        pack
        infra.util
        infra.stream
        hal.interfaces
        crypto.mbed_tls
        crypto.micro_ecc
        crypto.tiny_aes128
    )

ccola_end_component()

ccola_subdirectories(
    test
)
