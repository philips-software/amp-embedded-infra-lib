add_library(upgrade.pack_builder STATIC)
emil_build_for(upgrade.pack_builder HOST All PREREQUISITE_BOOL EMIL_STANDALONE)

target_link_libraries(upgrade.pack_builder PUBLIC
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
    ImageEncryptorNone.cpp
    ImageEncryptorNone.hpp
    ImageSecurity.hpp
    ImageSigner.hpp
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

if (EMIL_INCLUDE_MBEDTLS OR NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
    if (NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
        target_link_libraries(upgrade.pack_builder PUBLIC
            ${EMIL_EXTERNAL_MBEDTLS_TARGET}
        )
    else()
        target_link_libraries(upgrade.pack_builder PUBLIC
            mbedcrypto
        )
    endif()

    target_sources(upgrade.pack_builder PRIVATE
        ImageAuthenticatorHmac.cpp
        ImageAuthenticatorHmac.hpp
        ImageEncryptorAes.cpp
        ImageEncryptorAes.hpp
        ImageSignerEcDsa.hpp
        ImageSignerHashOnly.cpp
        ImageSignerHashOnly.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_helper)
