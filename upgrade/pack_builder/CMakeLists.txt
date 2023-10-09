add_library(upgrade.pack_builder STATIC)
emil_build_for(upgrade.pack_builder HOST All PREREQUISITE_BOOL EMIL_STANDALONE)

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

add_subdirectory(test)
add_subdirectory(test_helper)
