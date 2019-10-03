#include "upgrade/pack_builder/InputBinary.hpp"

namespace application
{
    InputBinary::InputBinary(const std::string& targetName, const std::string& fileName, uint32_t destinationAddress,
        hal::FileSystem& fileSystem, const ImageSecurity& imageSecurity)
        : Input(targetName)
        , destinationAddress(destinationAddress)
        , imageSecurity(imageSecurity)
        , image(fileSystem.ReadBinaryFile(fileName))
    {}

    InputBinary::InputBinary(const std::string& targetName, const std::vector<uint8_t>& contents, uint32_t destinationAddress,
        const ImageSecurity& imageSecurity)
        : Input(targetName)
        , destinationAddress(destinationAddress)
        , imageSecurity(imageSecurity)
        , image(contents)
    {}

    std::vector<uint8_t> InputBinary::Image() const
    {
        std::string targetName(TargetName());

        std::vector<uint8_t> result;
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(targetName.data()), reinterpret_cast<const uint8_t*>(targetName.data() + targetName.size()));
        result.insert(result.end(), Input::maxNameSize - targetName.size(), 0);

        uint32_t encryptionAndMacMethod = imageSecurity.EncryptionAndMacMethod();
        result.insert(result.end(), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod), reinterpret_cast<const uint8_t*>(&encryptionAndMacMethod + 1));

        std::vector<uint8_t> unencryptedImage;
        unencryptedImage.insert(unencryptedImage.end(), reinterpret_cast<const uint8_t*>(&destinationAddress), reinterpret_cast<const uint8_t*>(&destinationAddress + 1));

        uint32_t imageSize = image.size();
        unencryptedImage.insert(unencryptedImage.end(), reinterpret_cast<const uint8_t*>(&imageSize), reinterpret_cast<const uint8_t*>(&imageSize + 1));

        unencryptedImage.insert(unencryptedImage.end(), image.begin(), image.end());

        std::vector<uint8_t> encryptedImage = imageSecurity.Secure(unencryptedImage);
        result.insert(result.end(), encryptedImage.begin(), encryptedImage.end());

        uint32_t size = result.size() + sizeof(size);
        result.insert(result.begin(), reinterpret_cast<const uint8_t*>(&size), reinterpret_cast<const uint8_t*>(&size + 1));

        return result;
    }
}
