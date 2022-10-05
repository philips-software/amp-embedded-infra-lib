if (EMIL_HOST_BUILD)
    add_library(upgrade.pack_builder ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS upgrade.pack_builder EXPORT emilUpgradeTargets)

    target_link_libraries(upgrade.pack_builder PUBLIC
        mbedcrypto
        crypto.micro_ecc
        crypto.tiny_aes128
        hal.interfaces
        infra.syntax
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
        ImageSignerHashOnly.cpp
        ImageSignerHashOnly.hpp
        Input.cpp
        Input.hpp
        InputBinary.cpp
        InputBinary.hpp
        InputCommand.cpp
        InputCommand.hpp
        InputElf.cpp
        InputElf.hpp
        InputFactory.hpp
        InputHex.cpp
        InputHex.hpp
        SparseVector.hpp
        SupportedTargets.cpp
        SupportedTargets.hpp
        UpgradePackBuilder.cpp
        UpgradePackBuilder.hpp
        UpgradePackConfigParser.cpp
        UpgradePackConfigParser.hpp
        UpgradePackInputFactory.cpp
        UpgradePackInputFactory.hpp
    )

    if (EMIL_BUILD_TESTS)
        add_subdirectory(test)
    endif()

    if (BUILD_TESTING)
        add_subdirectory(test_helper)
    endif()
endif()
