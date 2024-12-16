#ifndef UPGRADE_UPGRADE_PACK_HEADER_HPP
#define UPGRADE_UPGRADE_PACK_HEADER_HPP

#include <array>
#include <cstdint>

namespace application
{
    enum class UpgradePackStatus : uint8_t
    {
        empty = 0xff,         // 1111.1111
        downloaded = 0xfe,    // 1111.1110
        readyToDeploy = 0xfc, // 1111.1100
        deployStarted = 0xf8, // 1111.1000
        deployed = 0xf0,      // 1111.0000
        invalid = 0x7f,       // 0111.1111
    };

    inline UpgradePackStatus operator|(UpgradePackStatus left, UpgradePackStatus right)
    {
        return static_cast<UpgradePackStatus>(static_cast<uint8_t>(left) | static_cast<uint8_t>(right));
    }

    static const std::array<uint8_t, 3> upgradePackMagic = { 'U', 'P', 'H' };

    static const uint32_t upgradeErrorCodeUnknownHeaderVersion = 0;
    static const uint32_t upgradeErrorCodeInvalidSignature = 1;
    static const uint32_t upgradeErrorCodeUnknownProductName = 2;
    static const uint32_t upgradeErrorCodeNoOrIncorrectSecondStageFound = 3;
    static const uint32_t upgradeErrorCodeNoSuitableImageUpgraderFound = 4;
    static const uint32_t upgradeErrorCodeImageUpgradeFailed = 5;
    static const uint32_t upgradeErrorCodeInvalidStartAddressOrStackPointer = 6;
    static const uint32_t upgradeErrorCodeExternalImageUpgradeFailed = 7;

    struct UpgradePackHeaderPrologue
    {
        UpgradePackStatus status;     // Overwritten after downloading, this indicates checked, invalid,
                                      // ready to deploy, deployed
        std::array<uint8_t, 3> magic; // Simple sanity check. Filled with 'U', 'P', 'H'.
        uint32_t errorCode;           // Set by the boot loaders upon detection of an error. 0-999 is reserved by the reference boot loaders.
        uint32_t signedContentsLength;

        uint16_t signatureMethod; // Identifier that indicates the signature method chosen. 1 = ECDSA-224, 2 = ECDSA-256.
        uint16_t signatureLength; // Size of the signature. 56 for ECDSA-224, 64 for ECDSA-256.
    };

    static_assert(sizeof(UpgradePackHeaderPrologue) == 16, "Incorrect size");

    struct UpgradePackHeaderEpilogue
    {
        uint16_t headerVersion; // Indicates the structure of the UpgradePackHeaders and ImageHeaderPrologue
        uint16_t headerLength;  // sizeof(UpgradePackHeaderPrologue) + signatureLength + sizeof(UpgradePackHeaderEpilogue)
        uint32_t numberOfImages;
        std::array<char, 64> productName;    // Product-specific name, checked by bootloader in order to avoid
                                             // flashing product 'A' with product 'B' firmware
        std::array<char, 64> productVersion; // Product-specific version string
        std::array<char, 64> componentName;  // Component name
        uint32_t componentVersion;           // Component version
    };

    static_assert(sizeof(UpgradePackHeaderEpilogue) == 204, "Incorrect size");

    struct ImageHeaderPrologue
    {
        uint32_t lengthOfHeaderAndImage; // sizeof(ImageHeaderPrologue) + binaryLength rounded up to multiple of 4,
                                         // filled out with zeros.
        std::array<char, 8> targetName;  // Indication for which upgrader to use. Some targetNames may be product-specific.
                                         // Generic: "boot1st", "boot2nd", "app", filled out with zeros.
        uint32_t encryptionAndMacMethod; // Identifier that indicates the encryption and MAC method chosen. 1 = aes.
        // uint8_t[depends on encryptionAndMacMethod] macAndIV;
        //// Start of encryption/mac
    };

    struct ImageHeaderEpilogue
    {
        uint32_t destinationAddress; // Address at which to flash the binary image
        uint32_t imageSize;          // Length of the binary image
    };
}

#endif
