if (TARGET_ARCH_X86)
    add_library(upgrade.pack_builder EXCLUDE_FROM_ALL STATIC)
    add_library(emil::upgrade.pack_builder ALIAS upgrade.pack_builder)
    install(TARGETS upgrade.pack_builder EXPORT emilUpgradeHostTargets)

    target_link_libraries(upgrade.pack_builder PUBLIC
        mbedcrypto
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
        BuildUpgradePack.cpp
        BuildUpgradePack.hpp
        Elf.hpp
        ImageAuthenticatorHmac.cpp
        ImageAuthenticatorHmac.hpp
        ImageEncryptorAes.cpp
        ImageEncryptorAes.hpp
        ImageEncryptorNone.cpp
        ImageEncryptorNone.hpp
        ImageSecurity.hpp
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

    if (BUILD_TESTING)
        add_subdirectory(test)
    endif()
endif()
