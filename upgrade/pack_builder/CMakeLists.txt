if(TARGET_ARCH_X86)
    add_library(upgrade.pack_builder EXCLUDE_FROM_ALL STATIC)

    target_link_libraries(upgrade.pack_builder
        PUBLIC

        crypto.mbed_tls
        crypto.micro_ecc
        crypto.tiny_aes128
        hal.interfaces
        infra.util
        infra.stream
        upgrade.pack
    )

    target_sources(upgrade.pack_builder PRIVATE
        BinaryObject.cpp
        BinaryObject.hpp
        BuildReferenceUpgradePack.cpp
        BuildReferenceUpgradePack.hpp
        BuildUpgradePack.cpp
        BuildUpgradePack.hpp
        Elf.hpp
        ImageAuthenticatorHmac.hpp
        ImageAuthenticatorHmac.cpp
        ImageEncryptorAes.cpp
        ImageEncryptorAes.hpp
        ImageEncryptorNone.cpp
        ImageEncryptorNone.hpp
        ImageEncryptorXtea.cpp
        ImageEncryptorXtea.hpp
        ImageSecurity.hpp
        ImageSecurityXteaHmac.cpp
        ImageSecurityXteaHmac.hpp
        ImageSigner.hpp
        ImageSignerEcDsa.cpp
        ImageSignerEcDsa.hpp
        ImageSignerRsa.cpp
        ImageSignerRsa.hpp
        Input.cpp
        Input.hpp
        InputBinary.cpp
        InputBinary.hpp
        InputElf.cpp
        InputElf.hpp
        InputFactory.hpp
        InputHex.cpp
        InputHex.hpp
        SparseVector.hpp
        UpgradePackBuilder.cpp
        UpgradePackBuilder.hpp
        UpgradePackConfigParser.cpp
        UpgradePackConfigParser.hpp
        UpgradePackInputFactory.cpp
        UpgradePackInputFactory.hpp
    )

    add_subdirectory(test)
endif()
